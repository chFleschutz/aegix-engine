#pragma once

#include "graphics/resources/static_mesh.h"

namespace Aegis::Graphics
{
	class MeshPreprocessor
	{
	public:
		struct Input
		{
			std::vector<glm::vec3> positions; // Required
			std::vector<glm::vec3> normals;   // Required
			std::vector<glm::vec2> uvs;
			std::vector<glm::vec3> colors;
			std::vector<uint32_t> indices;

			float overdrawThreshold = 1.05f;

			size_t maxVerticesPerMeshlet = 64;
			size_t maxTrianglesPerMeshlet = 126;
			float coneWeight = 0; 
		};

		static auto process(Input& input) -> StaticMesh::CreateInfo;

	private:
		static auto interleave(const Input& input) -> std::vector<StaticMesh::Vertex>;
	};
}