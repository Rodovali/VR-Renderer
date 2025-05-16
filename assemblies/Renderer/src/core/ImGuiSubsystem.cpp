// VR Renderer - ImGui Subsystem
// Rodolphe VALICON
// 2025

#include "ImGuiSubsystem.h"
#include "core/Logger.h"
#include "core/Input.h"
#include "event/EventDispatcher.h"
#include "event/InputEvents.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/glad.h>

namespace vr {
	
	ImGuiSubsystem::ImGuiSubsystem(Window& window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui_ImplGlfw_InitForOpenGL(window.getGLFWHandle(), true);
		ImGui_ImplOpenGL3_Init();

		logger::info("ImGui subsystem initialized.");
	}

	ImGuiSubsystem::~ImGuiSubsystem() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiSubsystem::beginFrame() const {
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiSubsystem::endFrame() const {
		glDisable(GL_DEBUG_OUTPUT);
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glEnable(GL_DEBUG_OUTPUT);
	}

	void ImGuiSubsystem::onEvent(const Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<events::KeyEvent>([](const events::KeyEvent& e) {
			bool wantCapture = ImGui::GetIO().WantCaptureKeyboard;
			if (wantCapture) input::clear();

			return wantCapture;
		});
		dispatcher.dispatch<events::MouseButtonEvent>([](const events::MouseButtonEvent& e) {
			bool wantCapture = ImGui::GetIO().WantCaptureMouse;
			if (wantCapture) input::clear();
			return wantCapture;
		});
	}

}

