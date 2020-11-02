#pragma once
#include "Engine/Context.h"
#include "Rendering/Vulkan/VulkanCommands.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	/** Handles large-scale synchronization of the rendering process, including drawing and presentation */
	struct VulkanSynchronizer {
		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;

		operator bool() const { return imageAvailableSemaphore && renderFinishedSemaphore; }

		static VulkanSynchronizer Create(CTX_ARG, VulkanLogicalDevice const& logical);
		void Destroy(VulkanLogicalDevice const& logical);

		/** Renders a single frame asynchronously. This will acquire an image, submit the commands for that image, then present the image. */
		bool RenderFrame(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, VulkanCommands const& commands);
	};
}
