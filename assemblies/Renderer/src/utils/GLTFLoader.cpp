// VR Renderer - GLTF 2.0 Loader
// Rodolphe VALICON
// 2025

#include "GLTFLoader.h"

#include "core/Logger.h"
#include "gpu/Texture.h"
#include "gpu/Sampler.h"
#include "renderer/MaterialInstance.h"
#include "renderer/MaterialRegistry.h"
#include "utils/Macros.h"
#include "utils/TangentCalculator.h"
#include "utils/ImageLoader.h"


#include <nlohmann/json.hpp>
#include <glm/glm.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>

using namespace nlohmann;

namespace vr {
	enum AttributeFlags : uint32_t {
		VA_POSITION = BIT(0),
		VA_TEXCOORDS = BIT(1),
		VA_NORMAL = BIT(2),
		VA_TANGENT = BIT(3),
	};

	static GLsizei getTypeSize(GLenum type) {
		switch (type) {
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:	return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_HALF_FLOAT:		return 2;
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:			return 4;
		case GL_DOUBLE:			return 8;
		default:				return 0;
		}
	}

	static const char* getTypeName(GLenum type) {
		switch (type) {
		case GL_BYTE:			return "i8";
		case GL_UNSIGNED_BYTE:	return "u8";
		case GL_SHORT:			return "i16";
		case GL_UNSIGNED_SHORT:	return "u16";
		case GL_HALF_FLOAT:		return "f16";
		case GL_INT:			return "i32";
		case GL_UNSIGNED_INT:	return "u32";
		case GL_FLOAT:			return "f32";
		case GL_DOUBLE:			return "f64";
		default: return "unknown";
		}
	}

	struct GLTFContext {
		std::string path;
		json content;
		mutable std::unordered_map<uint32_t, std::unique_ptr<uint8_t[]>> buffers;
		mutable std::unordered_map<uint32_t, std::shared_ptr<gpu::Texture>> textures;
		mutable std::unordered_map<uint32_t, std::shared_ptr<MaterialInstance>> materials;
		mutable std::unordered_map<uint32_t, gpu::Sampler> samplers;

		GLTFContext(const std::string& filePath) : path(filePath) {
			std::ifstream gltfFile(filePath);
			gltfFile >> content;
		}

		const uint8_t* getBuffer(uint32_t index) const {
			if (buffers.find(index) == buffers.end()) {
				// Parse buffer description
				const json& description = content["buffers"][index];
				size_t byteLength = description["byteLength"];
				std::string uri = description["uri"];

				// Read buffer
				std::filesystem::path binPath(path);
				binPath.replace_filename(uri);

				logger::debug("Reading glTF buffer {} at path '{}'", index, binPath.string());

				std::ifstream bufferFile(binPath.string(), std::ifstream::binary);
				std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(byteLength);
				bufferFile.read(reinterpret_cast<char*>(buffer.get()), byteLength);

				buffers[index] = std::move(buffer);
			}

			return buffers[index].get();
		}

		gpu::Sampler& getSampler(uint32_t index) const {
			if (samplers.find(index) == samplers.end()) {
				// Parse sampler description
				const json& description = content["samplers"][index];
				gpu::Sampler sampler{
					.magFilter = content.value("magFilter", GL_LINEAR),
					.minFilter = content.value("minFilter", GL_LINEAR),
					.wrapS = content.value("wrapS", GL_REPEAT),
					.wrapT = content.value("wrapT", GL_REPEAT)
				};

				samplers[index] = sampler;
			}

			return samplers[index];
		}

		std::shared_ptr<gpu::Texture> getTexture(uint32_t index, bool loadSRGB = false) const {
			if (textures.find(index) == textures.end()) {
				// Parse texture description
				const json& description = content["textures"][index];


				gpu::Sampler sampler;
				if (description.contains("sampler"))
					sampler = getSampler(description["sampler"]);

				std::string uri = content["images"][static_cast<uint32_t>(description["source"])]["uri"];

				// Read image
				std::filesystem::path imagePath(path);
				imagePath.replace_filename(uri);

				logger::debug("Reading glTF image {} at path '{}'", index, imagePath.string());
				
				std::shared_ptr<Image> image = utils::loadImage(imagePath.string(), GL_UNSIGNED_BYTE);

				GLenum format = 0;
				if (loadSRGB) {
					switch (image->pixelFormat) {
					case GL_RGB: format = GL_SRGB8; break;
					case GL_RGBA: format = GL_SRGB8_ALPHA8; break;
					}
				} else {
					switch (image->pixelFormat) {
					case GL_RED: format = GL_R8; break;
					case GL_RG: format = GL_RG8; break;
					case GL_RGB: format = GL_RGB8; break;
					case GL_RGBA: format = GL_RGBA8; break;
					}
				}

				auto texture = std::make_shared<gpu::Texture>(GL_TEXTURE_2D, sampler);
				glTextureParameterf(*texture, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);
				glTextureStorage2D(*texture, glm::log2(image->width), format, image->width, image->height);
				glTextureSubImage2D(*texture, 0, 0, 0, image->width, image->height, image->pixelFormat, image->pixelType, image->pixels.get());
				glGenerateTextureMipmap(*texture);

				textures[index] = texture;
			}

			return textures[index];
		}

		std::shared_ptr<MaterialInstance> getMaterial(uint32_t index) const {
			if (materials.find(index) == materials.end()) {
				// Parse material description
				logger::debug("Parsing Material {}", index);
				const json& description = content["materials"][index];
				const json& pbr = description["pbrMetallicRoughness"];
				auto material = std::make_shared<MaterialInstance>(MaterialRegistry::getMaterial("pbr"));
				
				glm::vec3 albedoFactor{};
				albedoFactor.r = pbr.value("/baseColorFactor/0"_json_pointer, 1.0f);
				albedoFactor.g = pbr.value("/baseColorFactor/1"_json_pointer, 1.0f);
				albedoFactor.b = pbr.value("/baseColorFactor/2"_json_pointer, 1.0f);

				float metallicFactor = pbr.value("metallicFactor", 1.0f);
				float roughnessFactor = pbr.value("roughnessFactor", 1.0f);

				glm::vec3 emissiveFactor{};
				emissiveFactor.r = description.value("/emissiveFactor/0"_json_pointer, 0.0f);
				emissiveFactor.g = description.value("/emissiveFactor/1"_json_pointer, 0.0f);
				emissiveFactor.b = description.value("/emissiveFactor/2"_json_pointer, 0.0f);

				material->set("MetallicFactor", metallicFactor);
				material->set("RoughnessFactor", roughnessFactor);
				material->set("AlbedoFactor", albedoFactor);
				material->set("EmissiveFactor", emissiveFactor);

				if (pbr.contains("baseColorTexture")) {
					material->set("AlbedoMap", 1);
					material->setTexture("sAlbedoMap", getTexture(pbr["/baseColorTexture/index"_json_pointer], true));
				} else {
					material->set("AlbedoMap", 0);
				}

				if (pbr.contains("metallicRoughnessTexture")) {
					material->set("MetalRoughnessMap", 1);
					material->setTexture("sMetalRoughnessMap", getTexture(pbr["/metallicRoughnessTexture/index"_json_pointer]));
				} else {
					material->set("MetalRoughnessMap", 0);
				}

				if (description.contains("normalTexture")) {
					material->set("NormalMap", 1);
					material->setTexture("sNormalMap", getTexture(description["/normalTexture/index"_json_pointer]));
				} else {
					material->set("NormalMap", 0);
				}

				if (description.contains("occlusionTexture")) {
					material->set("OcclusionMap", 1);
					material->setTexture("sOcclusionMap", getTexture(description["/occlusionTexture/index"_json_pointer]));
				} else {
					material->set("OcclusionMap", 0);
				}

				if (description.contains("emissiveTexture")) {
					material->set("EmissiveMap", 1);
					material->setTexture("sEmissiveMap", getTexture(description["/emissiveTexture/index"_json_pointer], true));
				} else {
					material->set("EmissiveMap", 0);
				}

				bool doubleSided = description.value("doubleSided", false);
				material->renderFlags.cullingEnable = !doubleSided;
				
				std::string alphaMode = description.value("alphaMode", "OPAQUE");
				if (alphaMode == "MASK") {
					float alphaCutoff = description.value("alphaCutoff", 0.5f);
					material->set("AlphaCutoff", alphaCutoff);
				} else {
					material->set("AlphaCutoff", 0.0f);
				}

				materials[index] = material;
			}

			return materials[index];
		}

	};

	struct BufferView {
		const uint8_t* buffer;
		size_t byteLength;
		size_t byteStride;
		GLenum target;

		BufferView() = default;
		BufferView(const GLTFContext& context, uint32_t id) {
			logger::debug("Constructing BufferView {}", id);
			json j = context.content["bufferViews"][id];
			
			size_t byteOffset = j.value("byteOffset", 0);
			buffer = context.getBuffer(j["buffer"]) + byteOffset;
			byteLength = j["byteLength"];
			byteStride = j.value("byteStride", 0);
			
			target = j.value("target", 0);
		}
	};

	struct Accessor {
		BufferView bufferView;
		size_t byteOffset;
		GLenum componentType;
		size_t count;
		uint32_t components;

		Accessor() = default;
		Accessor(const GLTFContext& context, uint32_t id) {
			const json& j = context.content["accessors"][id];
			logger::debug("Constructing Accessor {}", id);
			bufferView = BufferView(context, j["bufferView"]);
			byteOffset = j.value("byteOffset", 0);
			componentType = j["componentType"];
			count = j["count"];

			std::string type = j["type"];
			if (type == "SCALAR") components = 1;
			else if (type == "VEC2") components = 2;
			else if (type == "VEC3") components = 3;
			else if (type == "VEC4") components = 4;
			else if (type == "MAT2") components = 9;
			else if (type == "MAT4") components = 16;
			else {
				components = 0;
				logger::error("Unknown accessor type '{}' found", type);
			}
		}
	};

	static Primitive parsePrimitive(const GLTFContext& context, const json& mesh, uint32_t primitiveID) {
		logger::debug("Parsing primitive {}", primitiveID);
		auto geometry = std::make_shared<gpu::GeometryData>();
		
		const json& description = mesh["primitives"][primitiveID];

		// Load indices
		Accessor indexAccessor(context, description["indices"]);
		geometry->indices.resize(indexAccessor.count);

		const uint8_t* indexBuffer = indexAccessor.bufferView.buffer + indexAccessor.byteOffset;
		for (size_t k = 0; k < indexAccessor.count; ++k) {
			switch (indexAccessor.componentType) {
			case GL_UNSIGNED_SHORT:
			{
				uint16_t index;
				std::memcpy(&index, indexBuffer + k * sizeof(uint16_t), sizeof(uint16_t));
				geometry->indices[k] = index;
			} break;
			case GL_UNSIGNED_INT:
			{
				uint32_t index;
				std::memcpy(&index, indexBuffer + k * sizeof(uint32_t), sizeof(uint32_t));
				geometry->indices[k] = index;
			} break;
			default:
				logger::error("Unsuported index type encountered.");
				return {};
			}
		}

		// Parse attributes
		uint32_t attributeFlags = 0;
		const json& attributesJSON = description["attributes"];
		std::vector<Accessor> accessors;
		std::vector<gpu::VertexAttribute> vertexAttributes;
		size_t totalSize = 0;

		if (attributesJSON.contains("POSITION")) {
			Accessor accessor(context, attributesJSON["POSITION"]);
			vertexAttributes.push_back({ gpu::Attribute::Position, accessor.componentType, accessor.components });
			accessors.push_back(accessor);
			totalSize += getTypeSize(accessor.componentType) * accessor.components  * accessor.count;
			attributeFlags |= VA_POSITION;
		}

		if (attributesJSON.contains("TEXCOORD_0")) {
			Accessor accessor(context, attributesJSON["TEXCOORD_0"]);
			vertexAttributes.push_back({ gpu::Attribute::TexCoord0, accessor.componentType, accessor.components });
			accessors.push_back(accessor);
			totalSize += getTypeSize(accessor.componentType) * accessor.components * accessor.count;
			attributeFlags |= VA_TEXCOORDS;
		}

		if (attributesJSON.contains("NORMAL")) {
			Accessor accessor(context, attributesJSON["NORMAL"]);
			vertexAttributes.push_back({ gpu::Attribute::Normal, accessor.componentType, accessor.components });
			accessors.push_back(accessor);
			totalSize += getTypeSize(accessor.componentType) * accessor.components * accessor.count;
			attributeFlags |= VA_NORMAL;
		}

		if (attributesJSON.contains("TANGENT")) {
			// Ignore provided tangents if normals are not provided. (~3.7.2.1. Overview)
			if ((attributeFlags & VA_NORMAL)) {
				Accessor accessor(context, attributesJSON["TANGENT"]);
				vertexAttributes.push_back({ gpu::Attribute::Tangent, accessor.componentType, accessor.components });
				accessors.push_back(accessor);
				totalSize += getTypeSize(accessor.componentType) * accessor.components * accessor.count;
				attributeFlags |= VA_TANGENT;
			}
		}

		geometry->vertex_data.resize(totalSize);
		geometry->layout = gpu::VertexLayout(vertexAttributes);

		// Load data
		logger::debug("Constructing interleaved vertex buffer...");
		uint32_t i = 0;
		for (auto& [_, attribute] : geometry->layout) {
			Accessor& accessor = accessors[i];
			size_t attributeSize = getTypeSize(accessor.componentType) * accessor.components;
			const uint8_t* buffer = accessor.bufferView.buffer + accessor.byteOffset;

			size_t arrayStride = geometry->layout.getStride();

			// If bufferView is tightly packed, stride is the attributeSize, otherwise it's the bufferView's stride (~3.6.2.4. Data Alignment)
			bool packedBufferView = (accessor.bufferView.byteStride == 0);
			size_t accessorStride = packedBufferView ? attributeSize : accessor.bufferView.byteStride;

			logger::debug("Attribute: {} - {}[{}] ({} bytes)", attribute.offset, getTypeName(accessor.componentType), accessor.components, attributeSize);
			for (size_t k = 0; k < accessor.count; ++k) {
				std::memcpy(geometry->vertex_data.data() + k * arrayStride + attribute.offset, buffer + k * accessorStride, attributeSize);
			}

			++i;
		}
		logger::debug("Triangle count: {}", indexAccessor.count / 3);

		// TODO: Compute flat normals if they are not provided. (~3.7.2.1. Overview)
		
		// Compute tangents with MikkTSpace if they are not provided and all needed attributes are present. (~3.7.2.1. Overview)
		if (!(attributeFlags & VA_TANGENT) && (attributeFlags & VA_POSITION) && (attributeFlags & VA_TEXCOORDS) && (attributeFlags & VA_NORMAL)) {
			logger::debug("Generating tangents...");
			geometry = utils::computeTangents(geometry);
		}

		// Parse topology mode
		geometry->topology = description.value("mode", GL_TRIANGLES);

		// Load Material
		std::shared_ptr<MaterialInstance> material = context.getMaterial(description["material"]);

		// Construct primitive
		Primitive primitive;
		primitive.vertexArray = std::make_shared<gpu::VertexArray>(*geometry);
		primitive.material = material;

		return primitive;
	}

	static std::shared_ptr<Mesh> parseMesh(const GLTFContext& context, uint32_t meshIndex) {
		const json& description = context.content["meshes"][meshIndex];

		auto mesh = std::make_shared<Mesh>();
		mesh->primitives.reserve(description["primitives"].size());

		for (uint32_t i = 0; i < description["primitives"].size(); ++i) {
			auto primitive = parsePrimitive(context, description, i);
			mesh->primitives.push_back(primitive);
		}

		return mesh;
	}

	std::shared_ptr<Mesh> utils::loadGLTFMesh(const std::string& filePath, uint32_t meshIndex) {
		GLTFContext context(filePath);
		return parseMesh(context, meshIndex);
	}

}

