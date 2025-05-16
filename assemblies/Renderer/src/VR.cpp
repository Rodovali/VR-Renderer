// VR Renderer - Main Application
// Rodolphe VALICON
// 2025

#include "VR.h"

#include <iostream>

#include <imgui.h>
#include <GLFW/glfw3.h>

using namespace vr;

class GameApp final : public Application {
public:
	GameApp() : Application(1920, 1080, "VR Renderer") {
		m_camera.zNear = 0.01f;
		m_camera.zFar = 30.0f;
		m_cameraController.speed = 1.0f;

		// Load skyboxes
		m_skyboxes["Sunrise"] = std::make_shared<Skybox>("res/textures/skyboxes/qwantani_sunrise_4k.hdr", 0.5f);
		m_skyboxes["Night"] = std::make_shared<Skybox>("res/textures/skyboxes/lakeside_night_4k.hdr", 0.013f);
		m_skyboxes["Loft"] = std::make_shared<Skybox>("res/textures/skyboxes/photo_studio_loft_hall_4k.hdr", 0.5f);

		m_scene.skybox = m_skyboxes.begin()->second;

		// Sun/Moon
		m_scene.directionalLights.push_back(DirectionalLight{
			.direction = {-0.1f, -1.0f, -0.5f},
			.color = glm::vec3(1.0f, 1.0f, 1.0f),
			.power = 10.0f,
		});
		
		// Scene objects and lights
		// Light cube 1
		auto cube = utils::loadGLTFMesh("res/models/cube-emissive/Cube.gltf", 0);
		cube->transform.scale = glm::vec3(0.05f);
		cube->transform.translation = glm::vec3(0.5f, 0.1f, 0.0f);
		m_scene.meshes.push_back(cube);

		m_scene.pointLights.push_back(PointLight{
			.position = cube->transform.translation,
			.color = glm::vec3(1.0f, 0.0f, 0.0f),
			.power = 0.0f,
		});

		// Light cube 2
		cube = utils::loadGLTFMesh("res/models/cube-emissive/Cube.gltf", 0);
		cube->transform.scale = glm::vec3(0.05f);
		cube->transform.translation = glm::vec3(-0.5f, 0.1f, 0.0f);
		m_scene.meshes.push_back(cube);

		m_scene.pointLights.push_back(PointLight{
			.position = cube->transform.translation,
			.color = glm::vec3(0.0f, 1.0f, 0.0f),
			.power = 0.0f,
			});

		// Sponza scene
		auto sponza = utils::loadGLTFMesh("res/models/sponza/Sponza.gltf", 0);
		sponza->transform.scale = glm::vec3(0.002f); // Scene is huuuuuge.
		m_scene.meshes.push_back(sponza);

		// Damaged Helmet
		auto helmet = utils::loadGLTFMesh("res/models/helmet/DamagedHelmet.gltf", 0);
		helmet->transform.scale = glm::vec3(0.1f);
		helmet->transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		m_scene.meshes.push_back(helmet);

		// Initialize renderer and effects
		m_renderTarget = std::make_shared<RenderTarget>(1920, 1080, 4);
		m_renderer = std::make_unique<Renderer>(m_renderTarget);
		m_screenShader = std::make_unique<gpu::ShaderProgram>("res/shaders/renderpasses/screen.glsl");
		
		// Implemented lens dirt, but i find the effect rather bad. Maybe it's the fault of the dirt texture.
		std::shared_ptr<Image> lensImage = utils::loadImage("res/textures/lens_dirt.jpg", GL_UNSIGNED_BYTE);
		auto lensDirtTexture = std::make_shared<gpu::Texture>(GL_TEXTURE_2D);
		glTextureStorage2D(*lensDirtTexture, 1, GL_RGB8, lensImage->width, lensImage->height);
		glTextureSubImage2D(*lensDirtTexture, 0, 0, 0, lensImage->width, lensImage->height, lensImage->pixelFormat, lensImage->pixelType, lensImage->pixels.get());
		glGenerateTextureMipmap(*lensDirtTexture);
		m_bloom = std::make_unique<Bloom>(lensDirtTexture);

		// MSAA for main render pass.
		glEnable(GL_MULTISAMPLE);
		
		// Alpha Blending.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

protected:
	~GameApp() {
		logger::debug("Bye!");
	}

	virtual void onEvent(const vr::Event& event) override {
		EventDispatcher dispatcher(event);

		// On resize we need to resize the render target, and thus recreate the renderer
		dispatcher.dispatch<events::WindowResizeEvent>([this](const events::WindowResizeEvent& e) {
			m_renderTarget = std::make_shared<RenderTarget>(e.width, e.height, 4);
			m_renderer = std::make_unique<Renderer>(m_renderTarget);
			return false;
		});

		dispatcher.dispatch<events::KeyEvent>([this](const events::KeyEvent& e) {
			if (e.pressed && e.key == GLFW_KEY_ESCAPE) {
				// Stop app on escape
				stop();
			} else if (e.pressed && e.key == GLFW_KEY_R) {
				// Reload every shaders
				MaterialRegistry::reloadMaterials();
				m_screenShader->reload();
				m_bloom->reload();
			}
			return false;
		});
	}

	virtual void onUpdate(float deltaTime) override {
		// Handle camera movement
		m_cameraController.handle_input();
		m_cameraController.update(m_camera, deltaTime);

		// Update meshes & materials
		for (uint32_t i = 0; i < 2; ++i) {
			PointLight& light = m_scene.pointLights[i];

			// Sync cube and point light
			auto& cube = m_scene.meshes[i];
			cube->transform.translation = light.position;
			auto& cubeMat = cube->primitives[0].material;
			cubeMat->set("EmissiveFactor", light.color * light.power);
		}
		
		// Update lights
		// Makes the directional light frustum follow the main camera
		for (DirectionalLight& light : m_scene.directionalLights)
			light.computeMatrix(m_camera, m_directionalFrustumSize);


		// Model rotation with mouse
		static glm::vec2 modelRot(0.0f);
		if (input::isButtonDown(MouseButton::Right)) {
			float x, y;
			input::getMouseDelta(x, y);
			modelRot += glm::vec2(x, y) * 0.0002f;
		}

		glm::vec3 right = glm::normalize(glm::cross(m_camera.forward, m_camera.up));
		if (auto mesh = m_selectedMesh.lock()) {
			mesh->transform.rotation = glm::normalize(glm::angleAxis(modelRot.x, glm::vec3(0.0f, 1.0f, 0.0f)) * mesh->transform.rotation);
			mesh->transform.rotation = glm::normalize(glm::angleAxis(modelRot.y, right) * mesh->transform.rotation);
		}
		modelRot /= 1.1f;
	}

	virtual void onFixedUpdate() override {
		// TODO: Add physics here
	}

	virtual void onRender() override {
		m_renderer->beginScene(m_camera);
		m_renderer->submit(m_scene);
		m_renderer->endScene();
		if (m_bloomEnable)
			m_renderer->postprocess(*m_bloom);
		m_renderer->display(*m_screenShader);
	}

	virtual void onUI() override {
		ImGui::Begin("Options");
		if (ImGui::CollapsingHeader("Scene")) {
			static const char* current_item = m_skyboxes.begin()->first;
			if (ImGui::BeginCombo("Skybox", current_item)) {
				for (auto& [name, skybox] : m_skyboxes) {
					if (ImGui::Selectable(name)) {
						m_scene.skybox = skybox;
						current_item = name;
					}
				}
				ImGui::EndCombo();
			}
		}

		if (ImGui::CollapsingHeader("Camera")) {
			static float gamma = 2.2f;
			static float exposure = 1.0f;
			static float chromaticAbberation = 0.0f;
			static float saturation = 1.0f;
			static float luminosity = 1.0f;

			if (ImGui::SliderFloat("Gamma", &gamma, 0.0f, 3.0f))
				glProgramUniform1f(*m_screenShader, glGetUniformLocation(*m_screenShader, "uGamma"), gamma);
			if (ImGui::SliderFloat("Exposure", &exposure, 0.0f, 2.0f))
				glProgramUniform1f(*m_screenShader, glGetUniformLocation(*m_screenShader, "uExposure"), exposure);
			if (ImGui::SliderFloat("Chromatic aberration", &chromaticAbberation, 0.0f, 1.0f))
				glProgramUniform1f(*m_screenShader, glGetUniformLocation(*m_screenShader, "uChromaticAbberation"), chromaticAbberation);
			if (ImGui::SliderFloat("Saturation", &saturation, 0.0f, 2.0f))
				glProgramUniform1f(*m_screenShader, glGetUniformLocation(*m_screenShader, "uSaturation"), saturation);
			if (ImGui::SliderFloat("Luminosity", &luminosity, 0.0f, 2.0f))
				glProgramUniform1f(*m_screenShader, glGetUniformLocation(*m_screenShader, "uLuminosity"), luminosity);

			static int tonemapper = 0;
			static const char* tonemappers = "ACES Fitted\0ACES Filmic\0Reinhard\0Simple Exposure\0Disabled\0";
			if (ImGui::Combo("Tonemapper", &tonemapper, tonemappers))
				glProgramUniform1i(*m_screenShader, glGetUniformLocation(*m_screenShader, "uTonemapper"), tonemapper);
		}


		if (ImGui::CollapsingHeader("Bloom")) {
			static float bloomAmount = 0.5f;
			ImGui::Checkbox("Enable", &m_bloomEnable);
			if (ImGui::SliderFloat("Amount", &bloomAmount, 0.0f, 1.0f)) {
				m_bloom->setAmount(bloomAmount);
			}
		}


		if (ImGui::CollapsingHeader("Directional Lights")) {
			ImGui::SliderFloat("Frustum size", &m_directionalFrustumSize, 0.5f, 10.0f);
			
			int32_t i = 0;
			for (DirectionalLight& light : m_scene.directionalLights) {
				if (ImGui::TreeNode(std::format("Directional Light {}", i).c_str())) {
					ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&light.color));
					ImGui::DragFloat("Power", &light.power, 0.1f, 0.0f);
					ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&light.direction), 0.001f);
					ImGui::TreePop();
				}
			}
		}

		if (ImGui::CollapsingHeader("Point Lights")) {
			int32_t i = 0;
			for (PointLight& light : m_scene.pointLights) {
				if (ImGui::TreeNode(std::format("Point Light {}", i).c_str())) {
					ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&light.color));
					ImGui::DragFloat("Power", &light.power, 0.1f, 0.0f);
					ImGui::DragFloat3("Position", reinterpret_cast<float*>(&light.position), 0.001f);
					ImGui::DragFloat("Radius", &light.radius, 0.0001f, 0.0f, 1.0f, "%.5f");
					ImGui::TreePop();
				}
				++i;
			}
		}

		ImGui::End();

		ImGui::Begin("Meshes");
		int32_t i = 0;
		for (auto& mesh : m_scene.meshes) {
			if (ImGui::TreeNode(std::format("Mesh {}", i).c_str())) {
				ImGui::DragFloat3("Translation", glm::value_ptr(mesh->transform.translation), 0.01f);
				ImGui::DragFloat3("Scale", glm::value_ptr(mesh->transform.scale), 0.01f);

				if (ImGui::Button("Select for rotation")) {
					m_selectedMesh = mesh;
				}

				int32_t j = 0;
				for (auto& primitive : mesh->primitives) {
					if (ImGui::TreeNode(std::format("Material {}", j).c_str())) {
						primitive.material->immediateGUI();
						ImGui::TreePop();
					}
					++j;
				}

				ImGui::TreePop();
			}
			++i;
		}
		ImGui::End();
	}

private:
	Camera m_camera;
	CameraController m_cameraController;

	Scene m_scene;
	std::unordered_map<const char*, std::shared_ptr<Skybox>> m_skyboxes;
	std::weak_ptr<Mesh> m_selectedMesh;

	std::unique_ptr<gpu::ShaderProgram> m_screenShader;
	std::shared_ptr<RenderTarget> m_renderTarget;
	std::unique_ptr<Renderer> m_renderer;

	std::unique_ptr<Bloom> m_bloom;
	bool m_bloomEnable = true;

	float m_directionalFrustumSize = 5.0f;
};

vr::Application* vr::createApplication() {
	return new GameApp();
}