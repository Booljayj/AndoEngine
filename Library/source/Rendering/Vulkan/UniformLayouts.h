#pragma once
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct UniformLayouts {
		VkDescriptorSetLayout global = nullptr;
		VkDescriptorSetLayout object = nullptr;

		UniformLayouts(VkDevice device);
		UniformLayouts(UniformLayouts const&) = delete;
		UniformLayouts(UniformLayouts&&) noexcept = default;
		~UniformLayouts();

	private:
		MoveOnly<VkDevice> device;
	};
}
