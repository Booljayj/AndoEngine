#include "Engine/Logging.h"
#include "Rendering/Uniforms.h"
#include "Rendering/Vulkan/VulkanUniformLayouts.h"

namespace Rendering {
	bool VulkanUniformLayouts::Create(VulkanLogicalDevice const& logical) {
		//Create the uniform layout for global uniforms
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0] = GlobalUniforms::GetBinding();

			VkDescriptorSetLayoutCreateInfo layoutCI{};
			layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutCI.bindingCount = bindings.size();
			layoutCI.pBindings = bindings.data();

			assert(!global);
			if (vkCreateDescriptorSetLayout(logical.device, &layoutCI, nullptr, &global) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to create descriptor set layout");
				return false;
			}
		}
		//Create the uniform layout for object uniforms
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0] = ObjectUniforms::GetBinding();

			VkDescriptorSetLayoutCreateInfo layoutCI{};
			layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutCI.bindingCount = bindings.size();
			layoutCI.pBindings = bindings.data();

			assert(!object);
			if (vkCreateDescriptorSetLayout(logical.device, &layoutCI, nullptr, &object) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to create descriptor set layout");
				return false;
			}
		}
		return true;
	}

	void VulkanUniformLayouts::Destroy(VulkanLogicalDevice const& logical) {
		vkDestroyDescriptorSetLayout(logical.device, global, nullptr);
		vkDestroyDescriptorSetLayout(logical.device, object, nullptr);
		global = nullptr;
		object = nullptr;
	}
}
