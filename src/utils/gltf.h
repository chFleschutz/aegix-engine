#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <array>
#include <optional>

namespace Aegix::GLTF
{
	using vec3 = std::array<float, 3>;
	using vec4 = std::array<float, 4>;
	using quat = std::array<float, 4>;
	using mat4 = std::array<float, 16>;

	constexpr mat4 mat4Identity = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
	};



	struct Asset
	{
		std::string version;	// Required
		std::string generator;
		std::string minVersion;
		std::string copyright;
	};

	struct Scene
	{
		std::string name;
		std::vector<uint32_t> nodes;
	};

	struct Node
	{
		size_t camera;
		uint32_t mesh;
		std::vector<uint32_t> children;
		mat4 matrix = mat4Identity;
		quat rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		vec3 scale{ 1.0f };
		vec3 translation{ 0.0f };
		std::string name;

	};

	struct Primitive
	{
		enum Mode
		{
			Points = 0,
			Lines = 1,
			LineLoop = 2,
			LineStrip = 3,
			Triangles = 4,
			TriangleStrip = 5,
			TriangleFan = 6
		};

		std::map<std::string, uint32_t> attributes;	// Required
		uint32_t indices;
		uint32_t material;
		Mode mode = Triangles;
	};

	struct Mesh
	{

	};

	struct Accessor
	{
		enum ComponentType
		{
			Byte = 5120,
			UnsignedByte = 5121,
			Short = 5122,
			UnsignedShort = 5123,
			UnsignedInt = 5125,
			Float = 5126
		};

		enum Type
		{
			Scalar,
			Vec2,
			Vec3,
			Vec4,
			Mat2,
			Mat3,
			Mat4
		};

		uint32_t bufferView;
		uint32_t sparse;
		size_t byteOffset = 0;
		size_t count;				// Required
		ComponentType componentType;// Required
		Type type;					// Required
		std::vector<float> min;		// Size depends on type [1, 2, 3, 4, 9, 16]
		std::vector<float> max;		// Size depends on type [1, 2, 3, 4, 9, 16]
		std::string name;
		bool normalized = false;
	};

	struct BufferView
	{
		enum Target
		{
			ArrayBuffer = 34962,
			ElementArrayBuffer = 34963
		};

		uint32_t buffer;	// Required
		Target target;
		size_t byteLength;	// Required
		size_t byteOffset = 0;
		size_t byteStride;
		std::string name;
	};

	struct Buffer
	{
		size_t byteLength;	// Required
		std::string uri;
		std::string name;
	};


	struct GLTF
	{
		Asset asset;
		uint32_t defaultScene;
		std::vector<Scene> scenes;
		std::vector<Node> nodes;
		std::vector<Mesh> meshes;
		std::vector<Accessor> accessors;
		std::vector<BufferView> bufferViews;
		std::vector<Buffer> buffers;
	};
}
