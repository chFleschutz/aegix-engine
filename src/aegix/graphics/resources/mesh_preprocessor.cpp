#include "pch.h"
#include "mesh_preprocessor.h"

#include <meshoptimizer.h>

namespace Aegix::Graphics
{
	auto MeshPreprocessor::process(const Input& input) -> StaticMesh::CreateInfo
	{
		std::vector<StaticMesh::Vertex> initialVertices = interleave(input);
		size_t initialVertexCount = initialVertices.size();
		bool hasIndices = !input.indices.empty();
		size_t indexCount = hasIndices ? input.indices.size() : initialVertexCount;

		// Vertex and Index remapping

		std::vector<uint32_t> remap(initialVertexCount);
		size_t vertexCount = meshopt_generateVertexRemap(
			remap.data(),
			hasIndices ? input.indices.data() : nullptr,
			hasIndices ? input.indices.size() : initialVertexCount,
			initialVertices.data(),
			initialVertexCount,
			sizeof(StaticMesh::Vertex));

		std::vector<uint32_t> indices(indexCount);
		meshopt_remapIndexBuffer(
			indices.data(),
			hasIndices ? input.indices.data() : nullptr,
			hasIndices ? input.indices.size() : initialVertexCount,
			remap.data());

		std::vector<StaticMesh::Vertex> vertices(vertexCount);
		meshopt_remapVertexBuffer(
			vertices.data(),
			initialVertices.data(),
			initialVertexCount,
			sizeof(StaticMesh::Vertex),
			remap.data());

		// Optimizations

		meshopt_optimizeVertexCache(
			indices.data(),
			indices.data(),
			indices.size(),
			vertices.size());

		meshopt_optimizeOverdraw(
			indices.data(),
			indices.data(),
			indices.size(),
			&vertices[0].position.x,
			vertices.size(),
			sizeof(StaticMesh::Vertex),
			input.overdrawThreshold);

		meshopt_optimizeVertexFetch(
			vertices.data(),
			indices.data(),
			indices.size(),
			vertices.data(),
			vertices.size(),
			sizeof(StaticMesh::Vertex));

		// Meshlet generation

		size_t maxMeshlets = meshopt_buildMeshletsBound(
			indices.size(),
			input.maxVerticesPerMeshlet,
			input.maxTrianglesPerMeshlet);

		std::vector<meshopt_Meshlet> meshoptMeshlets(maxMeshlets);
		std::vector<uint32_t> meshletVertices(indexCount);
		std::vector<uint8_t> meshletPrimitives(indexCount);

		size_t meshletCount = meshopt_buildMeshlets(
			meshoptMeshlets.data(),
			meshletVertices.data(),
			meshletPrimitives.data(),
			indices.data(),
			indices.size(),
			&vertices[0].position.x,
			vertices.size(),
			sizeof(StaticMesh::Vertex),
			input.maxVerticesPerMeshlet,
			input.maxTrianglesPerMeshlet,
			input.coneWeight);

		const auto& lastMeshlet = meshoptMeshlets[meshletCount - 1];
		meshoptMeshlets.resize(meshletCount);
		meshletVertices.resize(lastMeshlet.vertex_offset + lastMeshlet.vertex_count);
		meshletPrimitives.resize(lastMeshlet.triangle_offset + lastMeshlet.triangle_count * 3);

		std::vector<StaticMesh::Meshlet> meshlets;
		meshlets.reserve(meshletCount);
		for (auto& meshlet : meshoptMeshlets)
		{
			meshopt_optimizeMeshlet(
				&meshletVertices[meshlet.vertex_offset],
				&meshletPrimitives[meshlet.triangle_offset],
				meshlet.triangle_count,
				meshlet.vertex_count);

			meshopt_Bounds bounds = meshopt_computeMeshletBounds(
				&meshletVertices[meshlet.vertex_offset],
				&meshletPrimitives[meshlet.triangle_offset],
				meshlet.triangle_count,
				&vertices[0].position.x,
				vertices.size(),
				sizeof(StaticMesh::Vertex));

			meshlets.emplace_back(StaticMesh::Meshlet{
				.center = { bounds.center[0], bounds.center[1], bounds.center[2] },
				.radius = bounds.radius,
				.coneAxis = { bounds.cone_axis_s8[0], bounds.cone_axis_s8[1], bounds.cone_axis_s8[2] },
				.coneCutoff = bounds.cone_cutoff_s8,
				.vertexOffset = meshlet.vertex_offset,
				.primitiveOffset = meshlet.triangle_offset,
				.vertexCount = static_cast<uint8_t>(meshlet.vertex_count),
				.primitiveCount = static_cast<uint8_t>(meshlet.triangle_count),
				});
		}

		// Fill CreateInfo

		StaticMesh::CreateInfo info{
			.vertexCount = static_cast<uint32_t>(vertices.size()),
			.indexCount = static_cast<uint32_t>(indices.size()),
			.meshletCount = static_cast<uint32_t>(meshlets.size()),
			.vertexIndexCount = static_cast<uint32_t>(meshletVertices.size()),
			.primitiveIndexCount = static_cast<uint32_t>(meshletPrimitives.size())
		};

		info.vertices = std::move(vertices);
		info.indices = std::move(indices);
		info.meshlets = std::move(meshlets);
		info.vertexIndices = std::move(meshletVertices);
		info.primitiveIndices = std::move(meshletPrimitives);
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
