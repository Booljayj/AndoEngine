#include "Rendering/Vulkan/VulkanFrameOrganizer.h"

namespace Rendering {
	bool VulkanFrameOrganizer::Create(CTX_ARG, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical, EBuffering buffering, size_t numImages, size_t numThreads) {
		size_t const numFrames = static_cast<size_t>(buffering) + 1;
		assert(numFrames > 0);

		frames.resize(numFrames);
		for (size_t index = 0; index < numFrames; ++index) {
			Frame& frame = frames[index];

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = physical.queues.graphics.value().index;
			poolInfo.flags = 0; // Optional

			assert(!frame.pool);
			if (vkCreateCommandPool(logical.device, &poolInfo, nullptr, &frame.pool) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create command pool for frame %i", index);
				return false;
			}

			VkCommandBufferAllocateInfo bufferAllocationInfo{};
			bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			bufferAllocationInfo.commandPool = frame.pool;
			bufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			bufferAllocationInfo.commandBufferCount = 1;

			assert(!frame.buffer);
			if (vkAllocateCommandBuffers(logical.device, &bufferAllocationInfo, &frame.buffer) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to allocate command buffers from the command pool for frame %i", index);
				return false;
			}

			VkSemaphoreCreateInfo semaphoreInfo{};
    		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &frame.imageAvailableSemaphore) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create image available semaphore for frame %i", index);
				return false;
			}
			if (vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &frame.renderFinishedSemaphore) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create render finished semaphore for frame %i", index);
				return false;
			}

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			if (vkCreateFence(logical.device, &fenceInfo, nullptr, &frame.fence) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create fence for frame %i", index);
				return false;
			}
		}

		imageFences.resize(numImages);

		return true;
	}

	void VulkanFrameOrganizer::Destroy(VulkanLogicalDevice const& logical) {
		for (Frame const& frame : frames) {
			if (frame.pool) vkDestroyCommandPool(logical.device, frame.pool, nullptr);
			if (frame.renderFinishedSemaphore) vkDestroySemaphore(logical.device, frame.renderFinishedSemaphore, nullptr);
			if (frame.imageAvailableSemaphore) vkDestroySemaphore(logical.device, frame.imageAvailableSemaphore, nullptr);
			if (frame.fence) vkDestroyFence(logical.device, frame.fence, nullptr);
		}
		frames.clear();
		currentFrameIndex = 0;
	}

	bool VulkanFrameOrganizer::Prepare(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, VulkanSwapImages const& images) {
		constexpr uint64_t waitTime = 5'000'000'000;

		Frame& frame = frames[currentFrameIndex];

		//Wait for the frame fence to finish, indicating this frame is no longer in use
		if (vkWaitForFences(logical.device, 1, &frame.fence, VK_TRUE, waitTime) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return false;
		}

		//Reset the command buffers used for this frame
		if (vkResetCommandPool(logical.device, frame.pool, 0) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Unable to reset command pool");
			return false;
		}

		//Acquire the swapchain image that can be used for this frame
		frame.currentImageIndex = 0;
		if (vkAcquireNextImageKHR(logical.device, swapchain.swapchain, waitTime, frame.imageAvailableSemaphore, VK_NULL_HANDLE, &frame.currentImageIndex) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for next available swapchain image");
			return false;
		}
		if (frame.currentImageIndex >= images.size() || frame.currentImageIndex >= imageFences.size()) {
			LOGF(Vulkan, Error, "Retrieved invalid image %i", frame.currentImageIndex);
			return false;
		}

		//If another frame is still using this image, wait for it to complete
		if (imageFences[frame.currentImageIndex] != VK_NULL_HANDLE) {
			if (vkWaitForFences(logical.device, 1, &imageFences[frame.currentImageIndex], VK_TRUE, waitTime) != VK_SUCCESS) {
				LOGF(Vulkan, Warning, "Timed out waiting for image fence %i", frame.currentImageIndex);
				return false;
			}
		}
		//Assign this frame's fence as the one using the swap image so other frames that try to use this image know to wait
		std::replace(imageFences.begin(), imageFences.end(), frame.fence, VkFence{VK_NULL_HANDLE});
		imageFences[frame.currentImageIndex] = frame.fence;

		//Reset the fence for this frame, so we can start another rendering process that it will track
		if (vkResetFences(logical.device, 1, &frame.fence) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Unable to reset image fence %i", frame.currentImageIndex);
			return false;
		}

		return true;
	}

	bool VulkanFrameOrganizer::Submit(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain) {
		Frame& frame = frames[currentFrameIndex];

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[1] = {frame.imageAvailableSemaphore};
		VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frame.buffer;

		VkSemaphore signalSemaphores[1] = {frame.renderFinishedSemaphore};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		//Submit the actual command buffer
		if (vkQueueSubmit(logical.queues.graphics, 1, &submitInfo, frame.fence) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to submit command buffer to qraphics queue");
			return false;
		}

		//Set up information for how to present the rendered image
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[1] = {swapchain.swapchain};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &frame.currentImageIndex;

		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(logical.queues.present, &presentInfo);

		//We've successfully finished rendering this frame, so move to the next frame
		currentFrameIndex = (currentFrameIndex + 1) % frames.size();
		return true;
	}
}
