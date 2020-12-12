#include "Rendering/Vertex.h"
#include "Engine/TupleUtility.h"

#define L_ATTRIBUTE(attribute, name, type) VkVertexInputAttributeDescription{ (uint8_t)EAttribute::attribute, 0, type, offsetof(ThisClass, name) }
#define L_VERIFY_PACKING(type)\
	static_assert(TupleElementSizes<type ## _Attributes>::Sum() == sizeof(type), #type " has unused padding space");\
	static_assert(sizeof(type) % sizeof(uint32_t) == 0, #type " must have a size that is a multiple of 4 bytes");\
	static_assert(alignof(type) == alignof(uint32_t), #type " has an invalid alignment")

namespace Rendering {
	using Vertex_Simple_Attributes = std::tuple<
		decltype(Vertex_Simple::position), decltype(Vertex_Simple::color),
		decltype(Vertex_Simple::normal), decltype(Vertex_Simple::uv0), decltype(Vertex_Simple::uv1), decltype(Vertex_Simple::uv2)
	>;
	L_VERIFY_PACKING(Vertex_Simple);

	VkVertexInputBindingDescription Vertex_Simple::GetBindingDescription() {
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = sizeof(Vertex_Simple);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
	}

	std::array<VkVertexInputAttributeDescription, 6> Vertex_Simple::GetAttributeDescriptions() {
		using ThisClass = Vertex_Simple;
		enum class EAttribute : uint8_t {
			Position, Color,
			Normal, UV0, UV1, UV2
		};

		return {
			L_ATTRIBUTE(Position, position, VK_FORMAT_R32G32B32_SFLOAT),
			L_ATTRIBUTE(Color, color, VK_FORMAT_R8G8B8A8_UNORM),

			L_ATTRIBUTE(Normal, normal, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(UV0, uv0, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV1, uv1, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV2, uv2, VK_FORMAT_R32_UINT),
		};
	}

	using Vertex_Complex_Attributes = std::tuple<
		decltype(Vertex_Complex::position), decltype(Vertex_Complex::color),
		decltype(Vertex_Complex::normal), decltype(Vertex_Complex::tangent), decltype(Vertex_Complex::bitangent), decltype(Vertex_Complex::userData),
		decltype(Vertex_Complex::uv0), decltype(Vertex_Complex::uv1), decltype(Vertex_Complex::uv2), decltype(Vertex_Complex::uv3)
	>;
	L_VERIFY_PACKING(Vertex_Complex);

	VkVertexInputBindingDescription Vertex_Complex::GetBindingDescription() {
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = sizeof(Vertex_Complex);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
	}

	std::array<VkVertexInputAttributeDescription, 10> Vertex_Complex::GetAttributeDescriptions() {
		using ThisClass = Vertex_Complex;
		enum class EAttribute : uint8_t {
			Position, Color,
			Normal, Tangent, Bitangent, UserData,
			UV0, UV1, UV2, UV3,
		};

		return {
			L_ATTRIBUTE(Position, position, VK_FORMAT_R32G32B32_SFLOAT),
			L_ATTRIBUTE(Color, color, VK_FORMAT_R8G8B8A8_UNORM),

			L_ATTRIBUTE(Normal, normal, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(Tangent, tangent, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(Bitangent, bitangent, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(UserData, userData, VK_FORMAT_R32_UINT),

			L_ATTRIBUTE(UV0, uv0, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV1, uv1, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV2, uv2, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV3, uv3, VK_FORMAT_R32_UINT),
		};
	}
}

#undef L_ATTRIBUTE
#undef L_VERIFY_PACKING
