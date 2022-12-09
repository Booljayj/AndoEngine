#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Uniforms.h"

namespace Rendering {
	bool VulkanFrameOrganizer::Create(VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical, VulkanUniformLayouts const& uniformLayouts, EBuffering buffering, size_t numImages, size_t numThreads) {
		uint32_t const numFrames = static_cast<size_t>(buffering) + 1;
		assert(numFrames > 0 && numFrames < 4);

		//Create the descriptor pool
		{
			//Each frame should have:
			//- One VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER (used by global uniforms)
			//- One VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC (used by object uniforms)
			//- Up to 16 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER (used by samplers in the global or object uniforms)
			//- Two descriptor sets (for global and object uniforms)

			std::array<VkDescriptorPoolSize, 3> poolSizes;
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = numFrames;
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			poolSizes[1].descriptorCount = numFrames;
			poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[2].descriptorCount = 16 * numFrames;

			VkDescriptorPoolCreateInfo descriptorPoolCI{};
			descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			descriptorPoolCI.pPoolSizes = poolSizes.data();
			descriptorPoolCI.maxSets = 2 * numFrames;

			assert(!descriptorPool);
			if (vkCreateDescriptorPool(logical.device, &descriptorPoolCI, nullptr, &descriptorPool) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to create descriptor pool for resources");
				return false;
			}
		}

		resources.resize(numFrames);
		for (size_t index = 0; index < numFrames; ++index) {
			FrameResources& frame = resources[index];

			//Create the command pool
			{
				VkCommandPoolCreateInfo poolInfo{};
				poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.queueFamilyIndex = physical.queues.graphics.value().index;
				poolInfo.flags = 0; // Optional

				if (vkCreateCommandPool(logical.device, &poolInfo, nullptr, &frame.commandPool) != VK_SUCCESS) {
					LOGF(Vulkan, Error, "Failed to create command pool for frame %i", index);
					return false;
				}
			}

			//Allocate the command buffer
			{
				VkCommandBufferAllocateInfo bufferAllocationInfo{};
				bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				bufferAllocationInfo.commandPool = frame.commandPool;
				bufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				bufferAllocationInfo.commandBufferCount = 1;

				if (vkAllocateCommandBuffers(logical.device, &bufferAllocationInfo, &frame.commands) != VK_SUCCESS) {
					LOGF(Vulkan, Error, "Failed to allocate command buffers from the command pool for frame %i", index);
					return false;
				}
			}

			//Create the uniforms descriptors and resources
			{
				VkDescriptorSet sets[] = { nullptr, nullptr };
				VkDescriptorSetLayout const layouts[] = { uniformLayouts.global, uniformLayouts.object };

				VkDescriptorSetAllocateInfo descriptorSetAI{};
				descriptorSetAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAI.descriptorPool = descriptorPool;
				descriptorSetAI.descriptorSetCount = 2;
				descriptorSetAI.pSetLayouts = layouts;

				if (vkAllocateDescriptorSets(logical.device, &descriptorSetAI, sets) != VK_SUCCESS) {
					LOGF(Vulkan, Error, "Could not allocate descriptor sets for frame %i", index);
					continue;
				}

				frame.uniforms.global.set = sets[0];
				frame.uniforms.object.set = sets[1];

				if (frame.uniforms.global.Reserve(logical.allocator, sizeof(GlobalUniforms), 1) == EResourceModifyResult::Error) {
					LOGF(Vulkan, Error, "Could not reserve space for global uniforms for frame %i", index);
				}
				frame.uniforms.global.UpdateDescriptors<false>(logical.device);

				if (frame.uniforms.object.Reserve(logical.allocator, sizeof(ObjectUniforms), 512) == EResourceModifyResult::Error) {
					LOGF(Vulkan, Error, "Could not reserve space for object uniforms for frame %i", index);
				}
				frame.uniforms.object.UpdateDescriptors<true>(logical.device);
			}

			//Create the semaphores
			{
				VkSemaphoreCreateInfo semaphoreInfo{};
				semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				if (vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &frame.semaphores.imageAvailable) != VK_SUCCESS) {
					LOGF(Vulkan, Error, "Failed to create image available semaphore for frame %i", index);
					return false;
				}
				if (vkCreateSemaphore(logical.device, &semaphoreInfo, nullptr, &frame.semaphores.renderFinished) != VK_SUCCESS) {
					LOGF(Vulkan, Error, "Failed to create render finished semaphore for frame %i", index);
					return false;
				}
			}

			//Create the fence
			{
				VkFenceCreateInfo fenceInfo{};
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				if (vkCreateFence(logical.device, &fenceInfo, nullptr, &frame.fence) != VK_SUCCESS) {
					LOGF(Vulkan, Error, "Failed to create fence for frame %i", index);
					return false;
				}
			}
		}

		imageFences.resize(numImages);

		//Create the generic command pool
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = physical.queues.graphics.value().index;
			poolInfo.flags = 0; // Optional

			assert(!commandPool);
			if (vkCreateCommandPool(logical.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to create general command pool");
				return false;
			}
		}

		//Allocate the staging command buffer
		{
			VkCommandBufferAllocateInfo bufferAllocationInfo{};
			bufferAllocationInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			bufferAllocationInfo.commandPool = commandPool;
			bufferAllocationInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			bufferAllocationInfo.commandBufferCount = 1;

			assert(!stagingCommandBuffer);
			if (vkAllocateCommandBuffers(logical.device, &bufferAllocationInfo, &stagingCommandBuffer) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to allocate general command buffers");
				return false;
			}
		}

		return true;
	}

	void VulkanFrameOrganizer::Destroy(VulkanLogicalDevice const& logical) {
		for (FrameResources const& frame : resources) {
			if (frame.commandPool) vkDestroyCommandPool(logical.device, frame.commandPool, nullptr);
			frame.uniforms.global.Destroy(logical.allocator);
			frame.uniforms.object.Destroy(logical.allocator);
			if (frame.semaphores.renderFinished) vkDestroySemaphore(logical.device, frame.semaphores.renderFinished, nullptr);
			if (frame.semaphores.imageAvailable) vkDestroySemaphore(logical.device, frame.semaphores.imageAvailable, nullptr);
			if (frame.fence) vkDestroyFence(logical.device, frame.fence, nullptr);
		}
		resources.clear();

		if (descriptorPool) vkDestroyDescriptorPool(logical.device, descriptorPool, nullptr);
		if (commandPool) vkDestroyCommandPool(logical.device, commandPool, nullptr);
		currentResourceIndex = 0;
	}

	EPreparationResult VulkanFrameOrganizer::Prepare(VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, size_t numObjects) {
		constexpr uint64_t waitTime = 5'000'000'000;

		FrameResources& frame = resources[currentResourceIndex];

		//Wait for the frame fence to finish, indicating this frame is no longer in use
		if (vkWaitForFences(logical.device, 1, &frame.fence, VK_TRUE, waitTime) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return EPreparationResult::Retry;
		}

		//Reset the command buffers used for this frame
		if (vkResetCommandPool(logical.device, frame.commandPool, 0) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Unable to reset command pool");
			return EPreparationResult::Error;
		}

		//Acquire the swapchain image that can be used for this frame
		currentImageIndex = 0;
		if (vkAcquireNextImageKHR(logical.device, swapchain.swapchain, waitTime, frame.semaphores.imageAvailable, VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for next available swapchain image");
			return EPreparationResult::Retry;
		}

		//Expand the fences array to make sure it contains the current index.
		if (currentImageIndex >= imageFences.size()) imageFences.resize(currentImageIndex + 1);

		//If another frame is still using this image, wait for it to complete
		if (imageFences[currentImageIndex] != VK_NULL_HANDLE) {
			if (vkWaitForFences(logical.device, 1, &imageFences[currentImageIndex], VK_TRUE, waitTime) != VK_SUCCESS) {
				LOGF(Vulkan, Warning, "Timed out waiting for image fence %i", currentImageIndex);
				return EPreparationResult::Retry;
			}
		}

		EResourceModifyResult const modifyResult = frame.uniforms.object.Reserve(logical.allocator, sizeof(ObjectUniforms), numObjects);
		switch (modifyResult) {
			case EResourceModifyResult::Error:
			return EPreparationResult::Error;

			case EResourceModifyResult::Modified:
			frame.uniforms.object.UpdateDescriptors<true>(logical.device);

			default: break;
		};

		//We're good to start recording commands
		return EPreparationResult::Success;
	}

	bool VulkanFrameOrganizer::Submit(VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain) {
		FrameResources const& frame = resources[currentResourceIndex];

		vmaFlushAllocation(logical.allocator, frame.uniforms.global.ubo.allocation, 0, VK_WHOLE_SIZE);
		vmaFlushAllocation(logical.allocator, frame.uniforms.object.ubo.allocation, 0, VK_WHOLE_SIZE);

		//Assign this frame's fence as the one using the swap image so other frames that try to use this image know to wait
		std::replace(imageFences.begin(), imageFences.end(), frame.fence, VkFence{VK_NULL_HANDLE});
		imageFences[currentImageIndex] = frame.fence;

		//Reset the fence for this frame, so we can start another rendering process that it will track
		if (vkResetFences(logical.device, 1, &frame.fence) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Unable to reset image fence %i", currentImageIndex);
			return false;
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore const waitSemaphores[] = { frame.semaphores.imageAvailable };
		VkPipelineStageFlags const waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		static_assert(std::size(waitSemaphores) == std::size(waitStages), "Number of semaphores must equal number of stages");
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(std::size(waitSemaphores));
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &frame.commands;

		VkSemaphore const signalSemaphores[] = { frame.semaphores.renderFinished };
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(std::size(signalSemaphores));
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

		VkSwapchainKHR const swapChains[] = { swapchain.swapchain };
		presentInfo.swapchainCount = static_cast<uint32_t>(std::size(swapChains));
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

	void VulkanFrameOrganizer::WaitForCompletion(VulkanLogicalDevice const& logical) {
		size_t const numFences = resources.size();
		t_vector<VkFence> fences{ numFences };
		for (size_t index = 0; index < resources.size(); ++index) {
			fences[index] = resources[index].fence;
		}

		vkWaitForFences(logical.device, static_cast<uint32_t>(numFences), fences.data(), VK_TRUE, std::numeric_limits<uint64_t>::max());
		vkDeviceWaitIdle(logical.device);
	}
}
