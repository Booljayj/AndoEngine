#pragma once
#include "Engine/StandardTypes.h"
#include "Rendering/Color.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	enum class EVertexType {
		Simple,
		Complex,
		SimpleSkinned,
		ComplexSkinned,
	};

	/** Pack and unpack UV coordinates using a 32-bit unsigned integer */
	inline uint32_t PackUV(glm::vec2 uv) { return glm::packHalf2x16(uv); }
	inline uint32_t PackUV(float u, float v) { return PackUV({u,v}); }

	/** Pack and unpack normalized vectors using a 32-bit unsigned integer */
	inline uint32_t PackNormal(glm::vec3 vector) {
		//Pack using the 2_10_10_10 format, 10-bit precision on x, y, and z with an unused 2-bit w component.
		uint32_t const x = std::roundf(std::clamp(vector.x, -1.0f, 1.0f) * 1023.0f);
		uint32_t const y = std::roundf(std::clamp(vector.y, -1.0f, 1.0f) * 1023.0f);
		uint32_t const z = std::roundf(std::clamp(vector.z, -1.0f, 1.0f) * 1023.0f);
		return (z << 20) | (y << 10) | x;
	}
	inline uint32_t PackNormal(float x, float y, float z) { return PackNormal({x,y,z}); }

	/** A simple 3D vertex */
	struct Vertex_Simple {
		glm::vec3 position; Color color;
		uint32_t normal; uint32_t uv0; uint32_t uv1; uint32_t uv2;

		enum class VariableIndices : uint8_t {
			Position, Color,
			Normal, UV0, UV1, UV2
		};
		using VariableTypes = std::tuple<
			decltype(position), decltype(color),
			decltype(normal), decltype(uv0), decltype(uv1), decltype(uv2)
		>;

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, std::tuple_size_v<VariableTypes>> GetAttributeDescriptions();

		Vertex_Simple()
		: position(0,0,0), color(0,0,0,255)
		, normal(PackNormal(0,0,1)), uv0(PackUV(0,0)), uv1(PackUV(0,0)), uv2(PackUV(0,0))
		{}

		Vertex_Simple(glm::vec3 inPosition, Color inColor, glm::vec3 inNormal, glm::vec2 inUV0, glm::vec2 inUV1 = {0,0}, glm::vec2 inUV2 = {0,0})
		: position(inPosition), color(inColor)
		, normal(PackNormal(inNormal)), uv0(PackUV(inUV0)), uv1(PackUV(inUV1)), uv2(PackUV(inUV2))
		{}
	};

	/** A complex 3d vertex, with tangent space vectors */
	struct Vertex_Complex {
		glm::vec3 position; Color color;
		uint32_t normal; uint32_t tangent; uint32_t bitangent; uint32_t userData;
		uint32_t uv0; uint32_t uv1; uint32_t uv2; uint32_t uv3;

		enum class VariableIndices : uint8_t {
			Position, Color,
			Normal, Tangent, Bitangent, UserData,
			UV0, UV1, UV2, UV3,
		};
		using VariableTypes = std::tuple<
			decltype(position), decltype(color),
			decltype(normal), decltype(tangent), decltype(bitangent), decltype(userData),
			decltype(uv0), decltype(uv1), decltype(uv2), decltype(uv3)
		>;

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, std::tuple_size_v<VariableTypes>> GetAttributeDescriptions();

		Vertex_Complex()
		: position(0,0,0), color(0,0,0,255)
		, normal(PackNormal(0,0,1)), tangent(PackNormal(1,0,0)), bitangent(PackNormal(0,1,0)), userData(0)
		, uv0(PackUV(0,0)), uv1(PackUV(0,0)), uv2(PackUV(0,0)), uv3(PackUV(0,0))
		{}
	};

	// /** A simple skinned 3D vertex */
	// struct Vertex_SimpleSkinned {
	// 	glm::vec3 position; Color color;
	// 	uint32_t normal; uint32_t uv0; uint32_t uv1; uint32_t uv2;
	// 	uint16_t boneIndices[4]; uint16_t boneWeights[4];

	// 	static VkVertexInputBindingDescription GetBindingDescription();
	// 	static std::array<VkVertexInputAttributeDescription, 8> GetAttributeDescriptions();

	// 	Vertex_SimpleSkinned()
	// 	: position(0,0,0), color(0,0,0,255)
	// 	, normal(PackNormal(0,0,1)), uv0(PackUV(0,0)), uv1(PackUV(0,0)), uv2(PackUV(0,0))
	// 	, boneIndices({UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX}), boneWeights({1,0,0,0})
	// 	{}
	// };

	// /** A complex skinned 3d vertex, with tangent space vectors */
	// struct Vertex_ComplexSkinned {
	// 	glm::vec3 position; Color color;
	// 	uint32_t normal; uint32_t tangent; uint32_t bitangent; uint32_t userData;
	// 	uint32_t uv0; uint32_t uv1; uint32_t uv2; uint32_t uv3;
	// 	uint16_t boneIndices[4]; uint16_t boneWeights[4];

	// 	static VkVertexInputBindingDescription GetBindingDescription();
	// 	static std::array<VkVertexInputAttributeDescription, 12> GetAttributeDescriptions();

	// 	Vertex_ComplexSkinned()
	// 	: position(0,0,0), color(0,0,0,255)
	// 	, normal(PackNormal(0,0,1)), tangent(PackNormal(1,0,0)), bitangent(PackNormal(0,1,0)), userData(0)
	// 	, uv0(PackUV(0,0)), uv1(PackUV(0,0)), uv2(PackUV(0,0)), uv3(PackUV(0,0))
	// 	, boneIndices({UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX}), boneWeights({1,0,0,0})
	// 	{}
	// };
}
