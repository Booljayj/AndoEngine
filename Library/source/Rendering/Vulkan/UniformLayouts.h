#pragma once
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct UniformLayouts {
		VkDescriptorSetLayout global = nullptr;
		VkDescriptorSetLayout object = nullptr;

		UniformLayouts(VkDevice device);
		UniformLayouts(UniformLayouts const&) = delete;
		UniformLayouts(UniformLayouts&&);
		~UniformLayouts();

	private:
		VkDevice device = nullptr;
	};
}
