// VR Renderer - Shader program system
// Rodolphe VALICON
// 2025

#include "ShaderProgram.h"

#include "core/Logger.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

static GLenum StringToShaderStage(std::string_view stageString) {
	if (stageString == "vertex")					return GL_VERTEX_SHADER;
	if (stageString == "tesselation_control")		return GL_TESS_CONTROL_SHADER;
	if (stageString == "tesselation_evaluation")	return GL_TESS_EVALUATION_SHADER;
	if (stageString == "geometry")					return GL_GEOMETRY_SHADER;
	if (stageString == "fragment")					return GL_FRAGMENT_SHADER;
	if (stageString == "compute")					return GL_COMPUTE_SHADER;

	return 0;
}

static const char* ShaderStageToString(GLenum shaderStage) {
	switch (shaderStage) {
	case GL_VERTEX_SHADER:			return "vertex";
	case GL_TESS_CONTROL_SHADER:	return "tesselation_control";
	case GL_TESS_EVALUATION_SHADER:	return "tesselation_evaluation";
	case GL_GEOMETRY_SHADER:		return "geometry";
	case GL_FRAGMENT_SHADER:		return "fragment";
	case GL_COMPUTE_SHADER:			return "compute";
	default:						return nullptr;
	}
}

namespace vr {
	namespace gpu {
		
		ShaderProgram::ShaderProgram(const char* path) : m_handle(0) {
			m_path = path;
			logger::info("Loading shader '{}'...", path);
			m_handle = loadProgram(path);
		}

		ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
			: m_handle(std::exchange(other.m_handle, 0)) {}

		ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
			if (m_handle == other.m_handle) return *this;

			glDeleteProgram(m_handle);
			m_handle = std::exchange(other.m_handle, 0);
			return *this;
		}

		ShaderProgram::~ShaderProgram() {
			glDeleteProgram(m_handle);
		}

		void ShaderProgram::reload() {
			logger::info("Reloading shader '{}'...", m_path);
			glDeleteProgram(m_handle);
			m_handle = loadProgram(m_path.c_str());
		}

		GLuint ShaderProgram::loadProgram(const char* path) {
			try {
				std::string source = readSource(path);
				SourceMap sources = preprocessSource(source);
				ShaderMap shaders = compileProgram(sources);
				return linkProgram(std::move(shaders));
			} catch (const std::runtime_error& error) {
				logger::error("Failed to load shader program '{}': {}", path, error.what());
				return 0;
			}
		}

		std::string ShaderProgram::readSource(const char* path) {
			std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
			if (!file) throw std::runtime_error("Failed to open file");

			std::string code;
			code.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(code.data(), code.size());

			return code;
		}

		ShaderProgram::SourceMap ShaderProgram::preprocessSource(const std::string& code) {
			SourceMap sources;
			std::stringstream codeStream(code);
			std::string commonCode = "";

			GLenum currentStage = GL_NONE;
			std::string currentStageCode = "";

			std::string line;
			uint32_t l = 0;
			while (std::getline(codeStream, line)) {
				++l;

				// In case of CRLF, remove trailing CR.
				if (line.back() == '\r')
					line.pop_back();

				// Detect stage directive
				if (line.substr(0, 6) == "#stage") {
					// Extract stage name and verify stage existence.
					std::string stageString = line.substr(7);
					GLenum stage = StringToShaderStage(stageString);
					if (!stage)
						throw std::runtime_error(std::format("Unknown stage '{}' encountered at line {}.", stageString, l));

					// If no stage was defined until now, collected code is common code.
					if (currentStage == GL_NONE) {
						commonCode = currentStageCode;
					} else {
						// Add commonCode + collected code to source map.
						// Warning if stage already defined.
						if (sources.find(currentStage) != sources.end())
							logger::warn("Shader stage '{}' is defined multiple times. Overwriting with the later definition.", stageString);

						// Remove trailing LF.
						currentStageCode.pop_back();
						sources[currentStage] = commonCode + currentStageCode;
					}

					// Update stage and reset collected code.
					currentStage = stage;
					currentStageCode = "";

					// Replace stage directive with a line directive at the right line.
					line = std::format("#line {}", l + 1);
				}

				// Collect line.
				currentStageCode += line + "\n";
			}

			// Adding last stage.
			if (currentStage == GL_NONE)
				throw std::runtime_error("No stage specified in the source file.");

			// Warning if stage already defined.
			if (sources.find(currentStage) != sources.end())
				logger::warn("Shader stage '{}' is defined multiple times. Overwriting with the later definition.", ShaderStageToString(currentStage));

			currentStageCode.pop_back();
			sources[currentStage] = commonCode + currentStageCode;

			return sources;
		}

		ShaderProgram::ShaderMap ShaderProgram::compileProgram(const ShaderProgram::SourceMap& sources) {
			ShaderMap shaders;

			for (const auto& [type, source] : sources) {
				// Create and compile shader
				GLuint shader = glCreateShader(type);
				const GLchar* code = source.data();
				GLint length = static_cast<GLint>(source.size());
				glShaderSource(shader, 1, &code, &length);
				glCompileShader(shader);

				// Report error if needed
				GLint success;
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (success == GL_FALSE) {
					GLint messageLength;
					glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &messageLength);

					std::string message;
					message.resize(messageLength);
					glGetShaderInfoLog(shader, messageLength, &messageLength, message.data());
					glDeleteShader(shader);

					throw std::runtime_error(std::format("Error compiling stage '{}'\n{}", ShaderStageToString(type), message));
				}

				// Add shader to shader map.
				shaders[type] = shader;
			}

			return shaders;
		}

		GLuint ShaderProgram::linkProgram(ShaderProgram::ShaderMap&& shaders) {
			GLuint program = glCreateProgram();

			// Link shaders
			for (auto& [stage, shader] : shaders) {
				glAttachShader(program, shader);
			}
			glLinkProgram(program);

			// Delete shaders
			for (auto& [_, shader] : shaders) {
				glDeleteShader(shader);
			}

			// Report error if needed
			GLint success;
			glGetProgramiv(program, GL_LINK_STATUS, &success);
			if (success == GL_FALSE) {
				GLint messageLength;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &messageLength);

				std::string message;
				message.resize(messageLength);
				glGetProgramInfoLog(program, messageLength, &messageLength, message.data());
				glDeleteProgram(program);

				throw std::runtime_error(std::format("Error linking program\n{}", message));
			}

			return program;
		}

	}
}
