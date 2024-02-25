#include "Rendering/Vulkan/UniformLayouts.h"
#include "Engine/Logging.h"
#include "Rendering/UniformTypes.h"

namespace Rendering {
	UniformLayouts::UniformLayouts(VkDevice device)
		: device(device)
	{
		//Create the uniform layout for global uniforms
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0] = GlobalUniforms::GetBinding();

			VkDescriptorSetLayoutCreateInfo const layoutCI{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = static_cast<uint32_t>(bindings.size()),
				.pBindings = bindings.data(),
			};
			
			if (vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &global) != VK_SUCCESS || !global) {
				throw std::runtime_error{ "Failed to create descriptor set layout" };
			}
		}
		//Create the uniform layout for object uniforms
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0] = ObjectUniforms::GetBinding();

			VkDescriptorSetLayoutCreateInfo const layoutCI{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.bindingCount = static_cast<uint32_t>(bindings.size()),
				.pBindings = bindings.data(),
			};
			
			if (vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &object) != VK_SUCCESS || !object) {
				throw std::runtime_error{ "Failed to create descriptor set layout" };
			}
		}
	}

	UniformLayouts::UniformLayouts(UniformLayouts&& other) noexcept
		: global(other.global), object(other.object), device(other.device)
	{
		other.device = nullptr;
	}

	UniformLayouts::~UniformLayouts() {
		if (device) {
			vkDestroyDescriptorSetLayout(device, global, nullptr);
			vkDestroyDescriptorSetLayout(device, object, nullptr);
		}
	}
}
