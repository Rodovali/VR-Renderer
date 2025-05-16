// VR Renderer - Shader program system
// Rodolphe VALICON
// 2025

#pragma once

#include <glad/glad.h>

#include <string>
#include <unordered_map>

namespace vr {
	namespace gpu {

		class ShaderProgram {
			using SourceMap = std::unordered_map<GLenum, std::string>;
			using ShaderMap = std::unordered_map<GLenum, GLuint>;

		public:
			ShaderProgram() : m_handle(0) {}
			ShaderProgram(const char* path);

			// No copy semantic
			ShaderProgram(const ShaderProgram&) = delete;
			ShaderProgram& operator=(const ShaderProgram&) = delete;

			// Move semantic
			ShaderProgram(ShaderProgram&& other) noexcept;
			ShaderProgram& operator=(ShaderProgram&& other) noexcept;

			~ShaderProgram();

			void reload();

			inline operator GLuint() const { return m_handle; }

		private:
			static GLuint loadProgram(const char* path);
			static std::string readSource(const char* path);
			static SourceMap preprocessSource(const std::string& code);
			static ShaderMap compileProgram(const SourceMap& sources);
			static GLuint linkProgram(ShaderMap&& shaders);

		private:
			GLuint m_handle;
			std::string m_path;
		};

	}
}
