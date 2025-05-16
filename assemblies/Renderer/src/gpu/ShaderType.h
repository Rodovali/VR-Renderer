// VR Renderer - Shader Types
// Rodolphe VALICON
// 2025

#pragma once

#include <glm/glm.hpp>

#include <cstdint>


namespace vr {
	namespace gpu {

		class ShaderType {
		public:
			enum Type : uint32_t {
				Bool = (1 << 0),
				Int = (1 << 2),
				Float = (1 << 3),
				Vec2 = (1 << 4),
				Color3 = (1 << 5),
				Vec3 = (1 << 6),
				Color4 = (1 << 7),
				Vec4 = (1 << 8),
				IVec2 = (1 << 9),
				IVec3 = (1 << 10),
				IVec4 = (1 << 11),
				Mat2 = (1 << 12),
				Mat3 = (1 << 13),
				Mat4 = (1 << 14),

				Unknown = 0
			};

		public:
			ShaderType() = default;
			ShaderType(Type type) : m_type(type) {}

			operator Type() const { return m_type; }
			explicit operator bool() const = delete;

			uint32_t size() const {
				switch (m_type) {
				case Bool | Int:
				case Bool:
				case Int:	return 4;
				case Float: return 4;
				case Vec2:	return 2 * 4;
				case Vec3 | Color3:
				case Color3:
				case Vec3:	return 3 * 4;
				case Vec4 | Color4:
				case Color4:
				case Vec4:	return 4 * 4;
				case IVec2: return 2 * 4;
				case IVec3: return 4 * 4;
				case IVec4: return 4 * 4;
				case Mat2:	return 2 * 2 * 4;
				case Mat3:	return 4 * 3 * 4;
				case Mat4:  return 4 * 4 * 4;
				default:	return 0;
				}
			}

			uint32_t alignment() const {
				switch (m_type) {
				case Bool | Int:
				case Bool:
				case Int:	return 4;
				case Float: return 4;
				case Vec2:	return 2 * 4;
				case Vec3 | Color3:
				case Color3:
				case Vec3:	return 4 * 4;
				case Vec4 | Color4:
				case Color4:
				case Vec4:	return 4 * 4;
				case IVec2: return 2 * 4;
				case IVec3: return 4 * 4;
				case IVec4: return 4 * 4;
				case Mat2:	return 2 * 4;
				case Mat3:	return 4 * 4;
				case Mat4:  return 4 * 4;
				default:	return 0;
				}
			}

			const char* name() const {
				switch (m_type) {
				case Bool:			return "Bool";
				case Int:			return "Int";
				case Int | Bool:	return "Int/Bool";
				case Float:			return "Float";
				case Vec2:			return "Vec2";
				case Vec3:			return "Vec3";
				case Color3:		return "Color3";
				case Vec3 | Color3:	return "Vec3/Color3";
				case Vec4:			return "Vec4";
				case Color4:		return "Color4";
				case Vec4 | Color4:	return "Vec4/Color4";
				case IVec2:			return "IVec2";
				case IVec3:			return "IVec3";
				case IVec4:			return "IVec4";
				case Mat2:			return "Mat2";
				case Mat3:			return "Mat3";
				case Mat4:			return "Mat4";
				default:			return "Unknown";
				}
			}

			static ShaderType fromName(std::string_view name) {
				if (name == "Bool") return ShaderType(ShaderType::Bool);
				if (name == "Int") return ShaderType(ShaderType::Int);
				if (name == "Float") return ShaderType(ShaderType::Float);
				if (name == "Vec2") return ShaderType(ShaderType::Vec2);
				if (name == "Color3") return ShaderType(ShaderType::Color3);
				if (name == "Vec3") return ShaderType(ShaderType::Vec3);
				if (name == "Color4") return ShaderType(ShaderType::Color4);
				if (name == "Vec4") return ShaderType(ShaderType::Vec4);
				if (name == "IVec2") return ShaderType(ShaderType::IVec2);
				if (name == "IVec3") return ShaderType(ShaderType::IVec3);
				if (name == "IVec4") return ShaderType(ShaderType::IVec4);
				if (name == "Mat2") return ShaderType(ShaderType::Mat2);
				if (name == "Mat3") return ShaderType(ShaderType::Mat3);
				if (name == "Mat4") return ShaderType(ShaderType::Mat4);

				return ShaderType(ShaderType::Unknown);
			}

			template<class T>
			static ShaderType associated_type() { return Type::Unknown; }

#define ASSOCIATE(type, shader_type) template<> static ShaderType associated_type<type>() { return shader_type; }
			ASSOCIATE(int32_t, (Type)(Int | Bool))
			ASSOCIATE(float, Float)
			ASSOCIATE(glm::vec2, Vec2)
			ASSOCIATE(glm::vec3, (Type)(Vec3 | Color3))
			ASSOCIATE(glm::vec4, (Type)(Vec4 | Color4))
			ASSOCIATE(glm::ivec2, IVec2)
			ASSOCIATE(glm::ivec3, IVec3)
			ASSOCIATE(glm::ivec4, IVec4)
			ASSOCIATE(glm::mat2, Mat2)
			ASSOCIATE(glm::mat3, Mat3)
			ASSOCIATE(glm::mat4, Mat4)
#undef ASSOCIATE

		private:
			Type m_type;
		};

	}
}
