#pragma once
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanSwapImages.h"

namespace Rendering {
	enum class EBuffering : uint8_t {
		None,
		Double,
		Triple,
	};

	/** Resources used for a single frame of rendering */
	struct Frame {
		/** Command recording objects */
		VkCommandPool pool = nullptr;
		VkCommandBuffer buffer = nullptr;

		/** Synchronization objects */
		VkSemaphore imageAvailableSemaphore = nullptr;
		VkSemaphore renderFinishedSemaphore = nullptr;
		VkFence fence = nullptr;

		/** The current image index being used by this frame. Can change each time a frame is retrieved. */
		uint32_t currentImageIndex = -1;

		static bool IsValid(Frame const& f) { return f.pool && f.buffer && f.imageAvailableSemaphore && f.renderFinishedSemaphore && f.fence; }
	};

	/** Keeps track of the resources used each frame, and how they should be used. */
	struct VulkanFrameOrganizer {
		std::vector<Frame> frames;
		std::vector<VkFence> imageFences;
		size_t currentFrameIndex = 0;

		inline operator bool() const { return frames.size() > 0 && std::all_of(frames.begin(), frames.end(), Frame::IsValid); }

		bool Create(CTX_ARG, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical, EBuffering buffering, size_t numImages, size_t numThreads);
		void Destroy(VulkanLogicalDevice const& logical);

		/** Prepare the next set of resources for rendering */
		bool Prepare(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, VulkanSwapImages const& images);
		/** Submit everything currently recorded so that it can be rendered. */
		bool Submit(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain);

		/** Record commands to the current buffer */
		template<typename FunctorType>
		inline bool Record(CTX_ARG, VulkanSwapImages const& images, FunctorType&& recorder) {
			Frame const& frame = frames[currentFrameIndex];
			SwapImage const& image = images[frame.currentImageIndex];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(frame.buffer, &beginInfo) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to begin recording command buffer");
				return false;
			}

			recorder(frame.buffer, image.framebuffer);

			if (vkEndCommandBuffer(frame.buffer) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to finish recording command buffer");
				return false;
			}

			return true;
		}
	};
}
