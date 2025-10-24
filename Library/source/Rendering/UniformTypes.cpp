#include "Rendering/UniformTypes.h"

namespace Rendering {
	VkDescriptorSetLayoutBinding GlobalUniforms::GetBinding() {
		return VkDescriptorSetLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS,
			.pImmutableSamplers = nullptr, // Optional
		};
	}

	VkDescriptorSetLayoutBinding ObjectUniforms::GetBinding() {
		return VkDescriptorSetLayoutBinding{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = nullptr, // Optional
		};
	}
}
