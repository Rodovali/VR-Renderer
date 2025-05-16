// VR Renderer - Tangent Calculator
// Rodolphe VALICON
// 2025

#include "TangentCalculator.h"

#include <mikktspace.h>
#include <weldmesh.h>

using namespace vr;

struct InOut {
	const gpu::GeometryData* inData;
	std::vector<float> outData;
};

static int getNumFaces(const SMikkTSpaceContext* pContext) {
	const gpu::GeometryData* data = reinterpret_cast<InOut*>(pContext->m_pUserData)->inData;

	return static_cast<int>(data->indices.size() / 3);
}

static int getNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace) {
	return 3;
}

static void getPosition(const SMikkTSpaceContext* pContext, float vfPosOut[], const int iFace, const int iVert) {
	const gpu::GeometryData* data = reinterpret_cast<InOut*>(pContext->m_pUserData)->inData;
	const uint32_t stride = data->layout.getStride();
	const uint32_t offset = data->layout.getAttribute(gpu::Attribute::Position).offset;

	const uint32_t index = data->indices[iFace * 3LL + iVert] * stride + offset;
	std::memcpy(vfPosOut, &data->vertex_data[index], 3 * sizeof(float));
}

static void getNormal(const SMikkTSpaceContext* pContext, float vfNormOut[], const int iFace, const int iVert) {
	const gpu::GeometryData* data = reinterpret_cast<InOut*>(pContext->m_pUserData)->inData;
	const uint32_t stride = data->layout.getStride();
	const uint32_t offset = data->layout.getAttribute(gpu::Attribute::Normal).offset;

	const uint32_t index = data->indices[iFace * 3LL + iVert] * stride + offset;
	std::memcpy(vfNormOut, &data->vertex_data[index], 3 * sizeof(float));
}

static void getTexCoord(const SMikkTSpaceContext* pContext, float vfTexcOut[], const int iFace, const int iVert) {
	const gpu::GeometryData* data = reinterpret_cast<InOut*>(pContext->m_pUserData)->inData;;
	const uint32_t stride = data->layout.getStride();
	const uint32_t offset = data->layout.getAttribute(gpu::Attribute::TexCoord0).offset;

	const uint32_t index = data->indices[iFace * 3LL + iVert] * stride + offset;
	std::memcpy(vfTexcOut, &data->vertex_data[index], 2 * sizeof(float));
}

static void setTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert) {
	const gpu::GeometryData* data = reinterpret_cast<InOut*>(pContext->m_pUserData)->inData;
	std::vector<float>& out = reinterpret_cast<InOut*>(pContext->m_pUserData)->outData;
	const uint32_t stride = data->layout.getStride();
	const uint32_t nFloats = stride / 4;

	const uint32_t newIndex = (iFace * 3 + iVert) * (nFloats + 4);
	const uint32_t oldIndex = data->indices[iFace * 3LL + iVert] * stride;

	// CONSTRUCT VERTEX
	// Copy vertex attributes
	std::memcpy(out.data() + newIndex, data->vertex_data.data() + oldIndex, stride);
	// Copy tangent and sign
	std::memcpy(out.data() + newIndex + nFloats, fvTangent, 3 * sizeof(float));
	std::memcpy(out.data() + newIndex + nFloats + 3, &fSign, sizeof(float));
}

namespace vr {
	std::shared_ptr<gpu::GeometryData> utils::computeTangents(std::shared_ptr<gpu::GeometryData> geometry) {
		const uint32_t nVertices = static_cast<uint32_t>(geometry->indices.size());
		const uint32_t nFloats = geometry->layout.getStride() / 4 + 4; // + Tangent vec3f + sign float

		// Compute tangents
		InOut data{};
		data.inData = geometry.get();
		data.outData = std::vector<float>(nVertices * nFloats);

		SMikkTSpaceInterface inter{};
		SMikkTSpaceContext context{};

		inter.m_getNumFaces = getNumFaces;
		inter.m_getNumVerticesOfFace = getNumVerticesOfFace;
		inter.m_getPosition = getPosition;
		inter.m_getNormal = getNormal;
		inter.m_getTexCoord = getTexCoord;
		inter.m_setTSpaceBasic = setTSpaceBasic;

		context.m_pInterface = &inter;
		context.m_pUserData = &data;

		genTangSpaceDefault(&context);

		// Construct new geometry
		std::shared_ptr<gpu::GeometryData> newGeometry = std::make_shared<gpu::GeometryData>();
		newGeometry->layout = geometry->layout;
		newGeometry->layout.addAttribute({ gpu::Attribute::Tangent, GL_FLOAT, 4 });
		newGeometry->indices.resize(nVertices);
		
		// Weld mesh
		std::unique_ptr<float[]> newVertexBuffer = std::make_unique<float[]>(nVertices * nFloats);
		uint32_t nNewVertices = WeldMesh(reinterpret_cast<int*>(newGeometry->indices.data()), newVertexBuffer.get(), data.outData.data(), nVertices, nFloats);
		newGeometry->vertex_data.resize(nNewVertices * nFloats * sizeof(float));
		std::memcpy(newGeometry->vertex_data.data(), newVertexBuffer.get(), nNewVertices * nFloats * sizeof(float));

		return newGeometry;
	}
}
