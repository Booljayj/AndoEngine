#include "Rendering/Vertex.h"
#include "Engine/TupleUtility.h"

#define L_ATTRIBUTE(attribute, name, type) VkVertexInputAttributeDescription{ (uint8_t)VariableIndices::attribute, 0, type, offsetof(ThisClass, name) }

namespace Rendering {
	VkVertexInputBindingDescription Vertex_Simple::GetBindingDescription() {
		return VkVertexInputBindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex_Simple),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		};
	}

	std::array<VkVertexInputAttributeDescription, Vertex_Simple::NumAttributes> Vertex_Simple::GetAttributeDescriptions() {
		using ThisClass = Vertex_Simple;
		
		return {
			L_ATTRIBUTE(Position, position, VK_FORMAT_R32G32B32_SFLOAT),
			L_ATTRIBUTE(Color, color, VK_FORMAT_R8G8B8A8_UNORM),

			L_ATTRIBUTE(Normal, normal, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(UV0, uv0, VK_FORMAT_R16G16_UINT),
			L_ATTRIBUTE(UV1, uv1, VK_FORMAT_R16G16_UINT),
			L_ATTRIBUTE(UV2, uv2, VK_FORMAT_R16G16_UINT),
		};
	}

	VkVertexInputBindingDescription Vertex_Complex::GetBindingDescription() {
		return VkVertexInputBindingDescription{
			.binding = 0,
			.stride = sizeof(Vertex_Complex),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		};
	}

	std::array<VkVertexInputAttributeDescription, Vertex_Complex::NumAttributes> Vertex_Complex::GetAttributeDescriptions() {
		using ThisClass = Vertex_Complex;
		
		return {
			L_ATTRIBUTE(Position, position, VK_FORMAT_R32G32B32_SFLOAT),
			L_ATTRIBUTE(Color, color, VK_FORMAT_R8G8B8A8_UNORM),

			L_ATTRIBUTE(Normal, normal, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(Tangent, tangent, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(Bitangent, bitangent, VK_FORMAT_A2B10G10R10_SNORM_PACK32),
			L_ATTRIBUTE(UserData, userData, VK_FORMAT_R32_UINT),

			L_ATTRIBUTE(UV0, uv0, VK_FORMAT_R16G16_UINT),
			L_ATTRIBUTE(UV1, uv1, VK_FORMAT_R16G16_UINT),
			L_ATTRIBUTE(UV2, uv2, VK_FORMAT_R16G16_UINT),
			L_ATTRIBUTE(UV3, uv3, VK_FORMAT_R16G16_UINT),
		};
	}
}

#undef L_ATTRIBUTE
#undef L_VERIFY_PACKING
