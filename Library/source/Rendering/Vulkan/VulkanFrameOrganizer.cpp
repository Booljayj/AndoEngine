#include "Rendering/Vulkan/VulkanFrameOrganizer.h"

namespace Rendering {
	bool VulkanFrameOrganizer::Create(CTX_ARG, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical, EBuffering buffering, size_t numImages, size_t numThreads) {
		size_t const numResources = static_cast<size_t>(buffering) + 1;
		assert(numResources > 0);

		resources.resize(numResources);
		for (size_t index = 0; index < numResources; ++index) {
			FrameResources& resource = resources[index];

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = physical.queues.graphics.value().index;
			poolInfo.flags = 0; // Optional

			assert(!resource.pool);
			if (vkCreateCommandPool(logical.device, &poolInfo, nullptr, &resource.pool) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create command pool for resource %i", index);
				return false;
			}

			VkCommandBufferAllocateInfo bufferAllocationInfo{};
			bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			bufferAllocationInfo.commandPool = resource.pool;
			bufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			bufferAllocationInfo.commandBufferCount = 1;

			assert(!resource.buffer);
			if (vkAllocateCommandBuffers(logical.device, &bufferAllocationInfo, &resource.buffer) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to allocate command buffers from the command pool for resource %i", index);
				return false;
			}

			VkSemaphoreCreateInfo semaphoreInfo{};
    		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &resource.imageAvailableSemaphore) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create image available semaphore for resource %i", index);
				return false;
			}
			if (vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &resource.renderFinishedSemaphore) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create render finished semaphore for resource %i", index);
				return false;
			}

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

			if (vkCreateFence(logical.device, &fenceInfo, nullptr, &resource.fence) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create fence for resource %i", index);
				return false;
			}
		}

		imageFences.resize(numImages);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = physical.queues.graphics.value().index;
		poolInfo.flags = 0; // Optional

		assert(!pool);
		if (vkCreateCommandPool(logical.device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create general command pool");
			return false;
		}

		VkCommandBufferAllocateInfo bufferAllocationInfo{};
		bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocationInfo.commandPool = pool;
		bufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAllocationInfo.commandBufferCount = 1;

		assert(!stagingCommandBuffer);
		if (vkAllocateCommandBuffers(logical.device, &bufferAllocationInfo, &stagingCommandBuffer) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to allocate general command buffers");
			return false;
		}

		return true;
	}

	void VulkanFrameOrganizer::Destroy(VulkanLogicalDevice const& logical) {
		for (FrameResources const& resource : resources) {
			if (resource.pool) vkDestroyCommandPool(logical.device, resource.pool, nullptr);
			if (resource.renderFinishedSemaphore) vkDestroySemaphore(logical.device, resource.renderFinishedSemaphore, nullptr);
			if (resource.imageAvailableSemaphore) vkDestroySemaphore(logical.device, resource.imageAvailableSemaphore, nullptr);
			if (resource.fence) vkDestroyFence(logical.device, resource.fence, nullptr);
		}
		resources.clear();
		if (pool) vkDestroyCommandPool(logical.device, pool, nullptr);
		currentResourceIndex = 0;
	}

	EPreparationResult VulkanFrameOrganizer::Prepare(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain) {
		constexpr uint64_t waitTime = 5'000'000'000;

		FrameResources const& resource = resources[currentResourceIndex];

		//Wait for the resource fence to finish, indicating this resource is no longer in use
		if (vkWaitForFences(logical.device, 1, &resource.fence, VK_TRUE, waitTime) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return EPreparationResult::Retry;
		}

		//Reset the command buffers used for this resource
		if (vkResetCommandPool(logical.device, resource.pool, 0) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Unable to reset command pool");
			return EPreparationResult::Error;
		}

		//Acquire the swapchain image that can be used for this resource
		currentImageIndex = 0;
		if (vkAcquireNextImageKHR(logical.device, swapchain.swapchain, waitTime, resource.imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for next available swapchain image");
			return EPreparationResult::Retry;
		}

		//Expand the fences array to make sure it contains the current index.
		if (currentImageIndex >= imageFences.size()) imageFences.resize(currentImageIndex + 1);

		//If another resource is still using this image, wait for it to complete
		if (imageFences[currentImageIndex] != VK_NULL_HANDLE) {
			if (vkWaitForFences(logical.device, 1, &imageFences[currentImageIndex], VK_TRUE, waitTime) != VK_SUCCESS) {
				LOGF(Vulkan, Warning, "Timed out waiting for image fence %i", currentImageIndex);
				return EPreparationResult::Retry;
			}
		}

		//We're good to start recording commands
		return EPreparationResult::Success;
	}

	bool VulkanFrameOrganizer::Submit(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain) {
		FrameResources const& resource = resources[currentResourceIndex];

		//Assign this resource's fence as the one using the swap image so other resources that try to use this image know to wait
		std::replace(imageFences.begin(), imageFences.end(), resource.fence, VkFence{VK_NULL_HANDLE});
		imageFences[currentImageIndex] = resource.fence;

		//Reset the fence for this resource, so we can start another rendering process that it will track
		if (vkResetFences(logical.device, 1, &resource.fence) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Unable to reset image fence %i", currentImageIndex);
			return false;
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[1] = {resource.imageAvailableSemaphore};
		VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &resource.buffer;

		VkSemaphore signalSemaphores[1] = {resource.renderFinishedSemaphore};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		//Submit the actual command buffer
		if (vkQueueSubmit(logical.queues.graphics, 1, &submitInfo, resource.fence) != VK_SUCCESS) {
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
		presentInfo.pImageIndices = &currentImageIndex;

		presentInfo.pResults = nullptr; // Optional

		if (vkQueuePresentKHR(logical.queues.present, &presentInfo) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to present image");
			return false;
		}

		//We've successfully finished rendering this resource, so move to the next resource
		currentResourceIndex = (currentResourceIndex + 1) % resources.size();
		return true;
	}

	void VulkanFrameOrganizer::WaitForCompletion(CTX_ARG, VulkanLogicalDevice const& logical) {
		size_t numFences = resources.size();
		VkFence* fences = CTX.temp.Request<VkFence>(numFences);
		for (size_t index = 0; index < resources.size(); ++index) {
			fences[index] = resources[index].fence;
		}

		vkWaitForFences(logical.device, numFences, fences, VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkDeviceWaitIdle(logical.device);
	}
}
