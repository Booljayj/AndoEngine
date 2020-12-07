#include "Rendering/Vertex.h"

#define L_ATTRIBUTE(attribute, name, type) VkVertexInputAttributeDescription{ 0, (uint8_t)EAttribute::attribute, type, offsetof(ThisClass, name) }

namespace Rendering {
	VkVertexInputBindingDescription Vertex_Simple::GetBindingDescription() {
		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = sizeof(Vertex_Simple);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return binding;
	}

	std::array<VkVertexInputAttributeDescription, 6> Vertex_Simple::GetAttributeDescriptions() {
		enum class EAttribute : uint8_t {
			Position, Color,
			Normal,
			UV0, UV1, UV2
		};

		return {
			L_ATTRIBUTE(Position, position, VK_FORMAT_R32G32B32_SFLOAT),
			L_ATTRIBUTE(Color, color, VK_FORMAT_R8G8B8A8_UINT),

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

	std::array<VkVertexInputAttributeDescription, 10> Vertex_Complex::GetAttributeDescriptions() {
		enum class EAttribute : uint8_t {
			Position, Color,
			Normal, Tangent, Bitangent, UserData,
			UV0, UV1, UV2, UV3,
		};

		return {
			L_ATTRIBUTE(Position, position, VK_FORMAT_R32G32B32_SFLOAT),
			L_ATTRIBUTE(Color, color, VK_FORMAT_R8G8B8A8_UINT),

			L_ATTRIBUTE(Normal, normal, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(Tangent, tangent, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(Bitangent, bitangent, VK_FORMAT_A2B10G10R10_SNORM_PACK32),

			L_ATTRIBUTE(UV0, uv0, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV1, uv1, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV2, uv2, VK_FORMAT_R32_UINT),
			L_ATTRIBUTE(UV3, uv3, VK_FORMAT_R32_UINT),
		};
	}
}

#undef L_ATTRIBUTE
