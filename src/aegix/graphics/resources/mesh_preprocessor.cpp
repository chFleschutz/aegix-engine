#include "pch.h"
#include "mesh_preprocessor.h"

#include <meshoptimizer.h>

namespace Aegix::Graphics
{
	auto MeshPreprocessor::process(const Input& input) -> StaticMesh::CreateInfo
	{
		StaticMesh::CreateInfo info{};
		auto vertices = interleave(input);

		// Vertex and Index remapping
		size_t indexCount = input.indices.size() == 0 ? vertices.size() : input.indices.size();
		const uint32_t* indexData = input.indices.size() == 0 ? nullptr : input.indices.data();

		std::vector<uint32_t> remapIndices(indexCount);
		size_t vertexCount = meshopt_generateVertexRemap(remapIndices.data(), indexData, indexCount, vertices.data(),
			vertices.size(), sizeof(StaticMesh::Vertex));

		info.indices.resize(indexCount);
		info.vertices.resize(vertexCount);
		info.indexCount = static_cast<uint32_t>(indexCount);
		info.vertexCount = static_cast<uint32_t>(vertexCount);

		meshopt_remapIndexBuffer(info.indices.data(), input.indices.data(), indexCount, remapIndices.data());
		meshopt_remapVertexBuffer(info.vertices.data(), vertices.data(), vertices.size(), sizeof(StaticMesh::Vertex),
			remapIndices.data());


		// Vertex optimizations

		meshopt_optimizeVertexCache(info.indices.data(), info.indices.data(), indexCount, vertexCount);
		meshopt_optimizeOverdraw(info.indices.data(), info.indices.data(), indexCount, &(info.vertices[0].position.x),
			vertexCount, sizeof(StaticMesh::Vertex), input.overdrawThreshold);
		meshopt_optimizeVertexFetch(info.vertices.data(), info.indices.data(), indexCount, info.vertices.data(),
			vertexCount, sizeof(StaticMesh::Vertex));


		// Meshlet generation

		size_t maxMeshlets = meshopt_buildMeshletsBound(indexCount, input.maxVerticesPerMeshlet,
			input.maxTrianglesPerMeshlet);

		std::vector<meshopt_Meshlet> meshlets(maxMeshlets);
		info.meshletIndices.resize(info.indexCount);
		info.meshletPrimitives.resize(info.indexCount);
		info.meshletCount = meshopt_buildMeshlets(meshlets.data(), info.meshletIndices.data(),
			info.meshletPrimitives.data(), info.indices.data(), info.indexCount, &(info.vertices[0].position.x),
			info.vertices.size(), sizeof(StaticMesh::Vertex), input.maxVerticesPerMeshlet, input.maxTrianglesPerMeshlet, 
			input.coneWeight);

		const auto& lastMeshlet = meshlets[info.meshletCount - 1];
		info.meshletIndexCount = lastMeshlet.triangle_offset + lastMeshlet.triangle_count;
		info.meshletPrimitiveCount = lastMeshlet.triangle_offset + lastMeshlet.triangle_count * 3;
		info.meshletIndices.resize(info.meshletIndexCount);
		info.meshletPrimitives.resize(info.meshletPrimitiveCount);
		meshlets.resize(info.meshletCount);

		info.meshlets.reserve(info.meshletCount);
		for (auto& meshlet : meshlets)
		{
			meshopt_Bounds bounds = meshopt_computeMeshletBounds(&info.meshletIndices[meshlet.vertex_offset],
				&info.meshletPrimitives[meshlet.triangle_offset], meshlet.triangle_count, &(info.vertices[0].position.x),
				info.vertexCount, sizeof(StaticMesh::Vertex));

			info.meshlets.emplace_back(StaticMesh::Meshlet{
				.vertexOffset = meshlet.vertex_offset,
				.vertexCount = meshlet.vertex_count,
				.primitiveOffset = meshlet.triangle_offset,
				.primitiveCount = meshlet.triangle_count,
				.center = glm::vec3{ bounds.center[0], bounds.center[1], bounds.center[2] },
				.radius = bounds.radius,
				.coneAxis = glm::vec3{ bounds.cone_axis[0], bounds.cone_axis[1], bounds.cone_axis[2] },
				.coneCutoff = bounds.cone_cutoff
				});
		}

		deinterleave(info);
		return info;
	}

	auto MeshPreprocessor::interleave(const Input& input) -> std::vector<StaticMesh::Vertex>
	{
		AGX_ASSERT_X(input.positions.size() == input.normals.size(), "Positions and normals size mismatch");

		std::vector<StaticMesh::Vertex> vertices(input.positions.size());
		for (size_t i = 0; i < input.positions.size(); i++)
		{
			vertices[i].position = input.positions[i];
			vertices[i].normal = input.normals[i];
			vertices[i].uv = (i < input.uvs.size()) ? input.uvs[i] : glm::vec2{ 0.0f };
			vertices[i].color = (i < input.colors.size()) ? input.colors[i] : glm::vec3{ 1.0f };
		}
		return vertices;
	}

	void MeshPreprocessor::deinterleave(StaticMesh::CreateInfo& info)
	{
		info.positions.reserve(info.vertexCount);
		info.normals.reserve(info.vertexCount);
		info.uvs.reserve(info.vertexCount);
		info.colors.reserve(info.vertexCount);
		for (const auto& vertex : info.vertices)
		{
			info.positions.emplace_back(vertex.position);
			info.normals.emplace_back(vertex.normal);
			info.uvs.emplace_back(vertex.uv);
			info.colors.emplace_back(vertex.color);
		}
	}
}
