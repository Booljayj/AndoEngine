#include "Rendering/Vulkan/VulkanSynchronizer.h"
#include "Engine/LogCommands.h"

namespace Rendering {
	VulkanSynchronizer VulkanSynchronizer::Create(CTX_ARG, VulkanLogicalDevice const& logical) {
		VulkanSynchronizer result;

		VkSemaphoreCreateInfo semaphoreInfo{};
    	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (
			vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &result.imageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &result.renderFinishedSemaphore) != VK_SUCCESS
		)
		{
			LOG(Vulkan, Error, "Failed to create semaphores");
			result.Destroy(logical);
		}

		return result;
	}

	void VulkanSynchronizer::Destroy(VulkanLogicalDevice const& logical) {
		if (renderFinishedSemaphore) {
			vkDestroySemaphore(logical.device, renderFinishedSemaphore, nullptr);
		}
		if (imageAvailableSemaphore) {
    		vkDestroySemaphore(logical.device, imageAvailableSemaphore, nullptr);
		}

		renderFinishedSemaphore = nullptr;
		imageAvailableSemaphore = nullptr;
	}

	bool VulkanSynchronizer::RenderFrame(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, VulkanCommands const& commands) {
		//Acquire the index of the next image that can be used
		uint32_t imageIndex;
		vkAcquireNextImageKHR(logical.device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		//Set up information about the command buffer to submit to the queue for this image
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[1] = {imageAvailableSemaphore};
		VkPipelineStageFlags waitStages[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commands.buffers[imageIndex];

		VkSemaphore signalSemaphores[1] = {renderFinishedSemaphore};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		//Submit the actual command buffer
		if (vkQueueSubmit(logical.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failedt to submit command buffer to qraphics queue");
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
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr; // Optional

		vkQueuePresentKHR(logical.queues.present, &presentInfo);

		//Temporary hard sync to wait for work to finish before moving to the next frame
		vkQueueWaitIdle(logical.queues.present);
		return true;
	}
}
