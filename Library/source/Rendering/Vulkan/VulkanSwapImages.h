#pragma once
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	/** Components for a single image in the swapchain */
	struct SwapImage {
		VkImageView view = nullptr;
		VkFramebuffer framebuffer = nullptr;

		static bool IsValid(SwapImage const& i) { return i.view && i.framebuffer; }
	};

	/** Contains components to use the images in a swapchain */
	struct VulkanSwapImages {
		std::vector<SwapImage> internal;

		inline operator bool() const { return internal.size() > 0 && std::all_of(internal.begin(), internal.end(), SwapImage::IsValid); }
		inline SwapImage const& operator[](size_t index) const { return internal[index]; }
		inline size_t size() const { return internal.size(); }

		bool Create(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain& swapchain, VulkanRenderPasses const& passes);
		void Destroy(VulkanLogicalDevice const& logical);
	};
}
