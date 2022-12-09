#include "Rendering/Vertex.h"
#include "Engine/TupleUtility.h"

#define L_ATTRIBUTE(attribute, name, type) VkVertexInputAttributeDescription{ (uint8_t)VariableIndices::attribute, 0, type, offsetof(ThisClass, name) }
#define L_VERIFY_PACKING(type, attributes)\
	static_assert(TupleElementSizes<attributes>::Sum() == sizeof(type), #type " has unused padding space");\
	static_assert(sizeof(type) % sizeof(uint32_t) == 0, #type " must have a size that is a multiple of 4 bytes");\
	static_assert(alignof(type) == alignof(uint32_t), #type " has an invalid alignment")

namespace Rendering {
	VkVertexInputBindingDescription Vertex_Simple::GetBindingDescription() {
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = sizeof(Vertex_Simple);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
	}

	std::array<VkVertexInputAttributeDescription, std::tuple_size_v<Vertex_Simple::VariableTypes>> Vertex_Simple::GetAttributeDescriptions() {
		using ThisClass = Vertex_Simple;
		constexpr size_t sizes = TupleElementSizes<VariableTypes>::Sum();
		L_VERIFY_PACKING(ThisClass, VariableTypes);

		return {
			L_ATTRIBUTE(Position, position, VK_FORMAT_R32G32B32_SFLOAT),
			L_ATTRIBUTE(Color, color, VK_FORMAT_R8G8B8A8_UNORM),

			L_ATTRIBUTE(Normal, normal, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(UV0, uv0, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV1, uv1, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV2, uv2, VK_FORMAT_R32_UINT),
		};
	}

	VkVertexInputBindingDescription Vertex_Complex::GetBindingDescription() {
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = sizeof(Vertex_Complex);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
	}

	std::array<VkVertexInputAttributeDescription, std::tuple_size_v<Vertex_Complex::VariableTypes>> Vertex_Complex::GetAttributeDescriptions() {
		using ThisClass = Vertex_Complex;
		L_VERIFY_PACKING(Vertex_Complex, VariableTypes);

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
