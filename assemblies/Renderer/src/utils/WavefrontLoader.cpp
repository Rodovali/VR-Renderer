// VR Renderer - Wavefront Object file loader
// Rodolphe VALICON
// 2025

#include "WavefrontLoader.h"

#include "core/Logger.h"

#include <fstream>
#include <string>
#include <optional>

#include <glm/glm.hpp>

struct Index {
	uint32_t p, t, n;
	bool operator==(const Index& other) { return p == other.p && t == other.t && n == other.n; }
};

struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct ParserLine {
	std::string_view view;
	size_t offset;
};

static std::optional<std::string_view> nextToken(ParserLine& line) {
	if (line.offset >= line.view.size()) return {};

	size_t startOffset;
	bool tokenStarted = false;
	do {
		if (!tokenStarted) {
			char c = line.view[line.offset];
			if (c == '\r') return {};
			if (c == ' ' || c == '/') {
				++line.offset;
				continue;
			}

			tokenStarted = true;
			startOffset = line.offset;
		} else {
			if (line.offset >= line.view.size() - 1) return line.view.substr(startOffset);
			char c = line.view[line.offset];
			if (c == ' ' || c == '/' || c == '\r') return line.view.substr(startOffset, line.offset - startOffset);
		}
		++line.offset;
	} while (true);

	return {};
}

template<class T>
static bool nextNumericToken(ParserLine& line, T& value) {
	auto token = nextToken(line);
	if (!token.has_value()) return false;

	auto [_, ec] = std::from_chars(token->data(), token->data() + token->size(), value);

	if (ec != std::errc())
		return false;

	return true;
}

namespace vr {

	std::shared_ptr<gpu::GeometryData> utils::loadWavefrontObj(const std::string& filePath) {
		std::ifstream file(filePath);
		if (!file) {
			logger::error("Failed to open Wavefront obj file '{}'.", filePath);
			return {};
		}

		// Prepare data vectors
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> uvs;

		std::vector<Index> faces;

		// Parsing variables
		uint32_t triangles = 0;

		std::string lineBuffer;
		std::optional<std::string_view> token;
		ParserLine line{};
		uint32_t l = 0;

		glm::vec3 p{};
		glm::vec3 n{};
		glm::vec2 t{};
		Index index{};

		// Parse file
		while (std::getline(file, lineBuffer)) {
			l++;
			line.view = lineBuffer;
			line.offset = 0;

			token = nextToken(line);

			if (!token.has_value()) continue;

			if (*token == "#") {
				// Comment
				continue;
			} else if (token == "o") {
				// Mesh name
				continue;
			} else if (token == "s") {
				// Smooth normals
				continue;
			} else if (token == "v") {
				// Vertex position
				if (!nextNumericToken(line, p.x) || !nextNumericToken(line, p.y) || !nextNumericToken(line, p.z)) {
					logger::error("Failed to parse Wavefront obj file '{}': Can't parse vertex position at line {}.", filePath, l);
					return {};
				}
				positions.push_back(p);
			} else if (token == "vn") {
				// Vertex normal
				if (!nextNumericToken(line, n.x) || !nextNumericToken(line, n.y) || !nextNumericToken(line, n.z)) {
					logger::error("Failed to parse Wavefront obj file '{}': Can't parse vertex normal at line {}.", filePath, l);
					return {};
				}
				normals.push_back(n);
			} else if (token == "vt") {
				// Vertex uv
				if (!nextNumericToken(line, t.x) || !nextNumericToken(line, t.y)) {
					logger::error("Failed to parse Wavefront obj file '{}': Can't parse vertex texture coordinates at line {}.", filePath, l);
				}
				uvs.push_back(t);
			} else if (token == "f") {
				// Face
				for (uint32_t i = 0; i < 3; ++i) {
					if (!nextNumericToken(line, index.p) || !nextNumericToken(line, index.t) || !nextNumericToken(line, index.n)) {
						logger::error("Failed to parse Wavefront obj file '{}': Can't parse face at line {}.", filePath, l);
						return {};
					}
					faces.push_back(index);
				}
				triangles++;
			} else {
				// Unknown
				logger::warn("Encountered an unknown token while parsing Wavefront obj file '{}' at line {}.", filePath, l);
			}
		}

		// Construct and weld mesh
		auto geometry = std::make_shared<gpu::GeometryData>();

		geometry->layout = {
			{ gpu::Attribute::Position, GL_FLOAT, 3 },
			{ gpu::Attribute::TexCoord0, GL_FLOAT, 2 },
			{ gpu::Attribute::Normal, GL_FLOAT, 3 },
		};

		std::vector<Index> indexRegistry;
		// registry is at most the size of the face indices.
		indexRegistry.reserve(faces.size());
		geometry->indices.reserve(faces.size());
		for (const Index& indexDesc : faces) {
			size_t index = std::distance(indexRegistry.begin(), std::find(indexRegistry.begin(), indexRegistry.end(), indexDesc));
			if (index == indexRegistry.size()) {
				indexRegistry.push_back(indexDesc);
			}
			geometry->indices.push_back(static_cast<uint32_t>(index));
		}

		geometry->vertex_data.resize(indexRegistry.size() * sizeof(Vertex));
		size_t offset = 0;
		for (const Index& indexDesc : indexRegistry) {
			Vertex* vertex = reinterpret_cast<Vertex*>(geometry->vertex_data.data() + offset);
			*vertex = {
				.position = positions[indexDesc.p - 1],
				.uv = uvs[indexDesc.t - 1],
				.normal = normals[indexDesc.n - 1]
			};
			offset += sizeof(Vertex);
		}

		geometry->topology = GL_TRIANGLES;

		logger::info("Loaded mesh ({} triangles) from '{}' Wavefront obj file.", triangles, filePath);

		return geometry;
	}
}