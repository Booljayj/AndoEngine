#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Uniforms.h"

namespace Rendering {
	FrameUniforms::FrameUniforms(VkDevice inDevice, VulkanUniformLayouts const& uniformLayouts, VkDescriptorPool pool, VmaAllocator allocator)
		: sets(inDevice, pool, { uniformLayouts.global, uniformLayouts.object })
		, global(inDevice, sets[0], 0, 1, allocator)
		, object(inDevice, sets[1], 0, 512, allocator)
	{}

	FrameUniforms::FrameUniforms(FrameUniforms&& other) noexcept
		: sets(std::move(other.sets))
		, global(std::move(other.global))
		, object(std::move(other.object))
	{}

	FrameSynchronization::FrameSynchronization(VkDevice inDevice)
		: device(inDevice)
	{
		//Create the semaphores
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		TUniqueHandle<vkDestroySemaphore> tempImageAvailable{ device };
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, *tempImageAvailable) != VK_SUCCESS || !tempImageAvailable) {
			throw std::runtime_error{ "Failed to create image available semaphore" };
		}
		TUniqueHandle<vkDestroySemaphore> tempRenderFinished{ device };
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, *tempRenderFinished) != VK_SUCCESS || !tempRenderFinished) {
			throw std::runtime_error{ "Failed to create render finished semaphore" };
		}

		//Create the fence
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS || !fence) {
			throw std::runtime_error{ "Failed to create fence" };
		}

		imageAvailable = tempImageAvailable.Release();
		renderFinished = tempRenderFinished.Release();
	}

	FrameSynchronization::FrameSynchronization(FrameSynchronization&& other) noexcept {
		std::swap(imageAvailable, other.imageAvailable);
		std::swap(renderFinished, other.renderFinished);
		std::swap(fence, other.fence);
		std::swap(device, other.device);
	}

	FrameSynchronization::~FrameSynchronization() {
		if (device) {
			vkWaitForFences(device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

			vkDestroySemaphore(device, renderFinished, nullptr);
			vkDestroySemaphore(device, imageAvailable, nullptr);
			vkDestroyFence(device, fence, nullptr);
		}
	}

	FrameResources::FrameResources(VkDevice inDevice, uint32_t graphicsQueueFamilyIndex, VulkanUniformLayouts const& uniformLayouts, VkDescriptorPool descriptorPool, VmaAllocator allocator)
		: uniforms(inDevice, uniformLayouts, descriptorPool, allocator)
		, mainCommandPool(inDevice, graphicsQueueFamilyIndex)
		, mainCommandBuffer(mainCommandPool.CreateBuffer(VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY))
		, sync(inDevice)
	{}

	FrameResources::FrameResources(FrameResources&& other) noexcept
		: uniforms(std::move(other.uniforms))
		, mainCommandPool(std::move(other.mainCommandPool))
		, mainCommandBuffer(other.mainCommandBuffer)
		, threadCommandPools(std::move(other.threadCommandPools))
		, threadCommandBuffers(std::move(other.threadCommandBuffers))
		, sync(std::move(other.sync))
	{}

	VulkanFrameOrganizer::VulkanFrameOrganizer(VulkanLogicalDevice const& logical, VulkanPhysicalDevice const& physical, VulkanSwapchain const& swapchain, VulkanUniformLayouts const& uniformLayouts, EBuffering buffering)
		: device(logical)
		, swapchain(swapchain)
		, queue({ logical.queues.graphics, logical.queues.present })
		, graphicsQueueFamilyIndex(physical.queues.graphics->index)
		, descriptorPool(device, GetPoolSizes(buffering), GetNumFrames(buffering) * 2)
	{
		size_t const numFrames = GetNumFrames(buffering);
		for (size_t index = 0; index < numFrames; ++index) {
			resources.emplace_back(device, graphicsQueueFamilyIndex, uniformLayouts, descriptorPool, logical.allocator);
		}
		imageFences.resize(swapchain.images.size(), nullptr);
	}

	VulkanFrameOrganizer::VulkanFrameOrganizer(VulkanFrameOrganizer&& other) noexcept
		: descriptorPool(std::move(other.descriptorPool))
		, resources(std::move(other.resources))
	{
		std::swap(device, other.device);
		std::swap(swapchain, other.swapchain);
		std::swap(queue.graphics, other.queue.graphics);
		std::swap(queue.present, other.queue.present);
		std::swap(graphicsQueueFamilyIndex, other.graphicsQueueFamilyIndex);

		std::swap(imageFences, other.imageFences);

		std::swap(currentResourceIndex, other.currentResourceIndex);
		std::swap(currentImageIndex, other.currentImageIndex);
	}

	std::optional<RecordingContext> VulkanFrameOrganizer::CreateRecordingContext(size_t numObjects, size_t numThreads) {
		constexpr uint64_t waitTime = 5'000'000'000;

		FrameResources& frame = resources[currentResourceIndex];

		//Wait for the frame fence to finish, indicating this frame is no longer in use
		if (vkWaitForFences(device, 1, &frame.sync.fence, VK_TRUE, waitTime) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return std::optional<RecordingContext>{};
		}

		//Reset the command pools so new commands can be written
		frame.mainCommandPool.Reset();
		for (CommandPool& threadCommandPool : frame.threadCommandPools) {
			threadCommandPool.Reset();
		}

		//Acquire the swapchain image that can be used for this frame
		currentImageIndex = 0;
		if (vkAcquireNextImageKHR(device, swapchain, waitTime, frame.sync.imageAvailable, VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for next available swapchain image");
			return std::optional<RecordingContext>{};
		}

		//Expand the fences array to make sure it contains the current index.
		if (currentImageIndex >= imageFences.size()) imageFences.resize(currentImageIndex + 1);

		//If another frame is still using this image, wait for it to complete
		if (imageFences[currentImageIndex] != VK_NULL_HANDLE) {
			if (vkWaitForFences(device, 1, &imageFences[currentImageIndex], VK_TRUE, waitTime) != VK_SUCCESS) {
				LOGF(Vulkan, Warning, "Timed out waiting for image fence %i", currentImageIndex);
				return std::optional<RecordingContext>{};
			}
		}

		//Resize the object uniforms buffer so we can store all the per-object data that will be used for rendering
		frame.uniforms.object.Reserve(numObjects);

		//Grow the number of command pools to match the number of threads
		for (size_t threadIndex = frame.threadCommandPools.size(); threadIndex < numThreads; ++threadIndex) {
			CommandPool& newPool = frame.threadCommandPools.emplace_back(device, graphicsQueueFamilyIndex);
			VkCommandBuffer& newBuffer = frame.threadCommandBuffers.emplace_back();

			newBuffer = newPool.CreateBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		}

		return RecordingContext{ currentImageIndex, currentResourceIndex, frame.uniforms, frame.mainCommandBuffer, frame.threadCommandBuffers };
	}

	void VulkanFrameOrganizer::Submit(TArrayView<VkCommandBuffer const> commands) {
		FrameResources const& frame = resources[currentResourceIndex];

		frame.uniforms.global.Flush();
		frame.uniforms.object.Flush();
		
		//Assign this frame's fence as the one using the swap image so other frames that try to use this image know to wait
		std::replace(imageFences.begin(), imageFences.end(), frame.sync.fence, VkFence{VK_NULL_HANDLE});
		imageFences[currentImageIndex] = frame.sync.fence;

		//Reset the fence for this frame, so we can start another rendering process that it will track
		if (vkResetFences(device, 1, &frame.sync.fence) != VK_SUCCESS) {
			throw std::runtime_error{ "Unable to reset image fence" };
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore const waitSemaphores[] = { frame.sync.imageAvailable };
		VkPipelineStageFlags const waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		static_assert(std::size(waitSemaphores) == std::size(waitStages), "Number of semaphores must equal number of stages");
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(std::size(waitSemaphores));
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = static_cast<uint32_t>(commands.size());
		submitInfo.pCommandBuffers = commands.begin();

		VkSemaphore const signalSemaphores[] = { frame.sync.renderFinished };
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(std::size(signalSemaphores));
		submitInfo.pSignalSemaphores = signalSemaphores;

		//Submit the actual command buffer
		if (vkQueueSubmit(queue.graphics, 1, &submitInfo, frame.sync.fence) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to submit command buffer to qraphics queue" };
		}

		//Set up information for how to present the rendered image
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR const swapchains[] = { swapchain };
		presentInfo.swapchainCount = static_cast<uint32_t>(std::size(swapchains));
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &currentImageIndex;

		presentInfo.pResults = nullptr; //Optional

		if (vkQueuePresentKHR(queue.present, &presentInfo) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to present image" };
		}

		//We've successfully finished rendering this resource, so move to the next resource
		currentResourceIndex = (currentResourceIndex + 1) % resources.size();
	}

	VulkanFrameOrganizer::PoolSizesType VulkanFrameOrganizer::GetPoolSizes(EBuffering buffering) {
		uint32_t const numFrames = static_cast<uint32_t>(GetNumFrames(buffering));

		//Each frame should have:
		//- One VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER (used by global uniforms)
		//- One VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC (used by object uniforms)
		//- Up to 16 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER (used by samplers in the global or object uniforms)
		//- Two descriptor sets (for global and object uniforms)
		return {
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, numFrames },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, numFrames },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, numFrames * 16 },
		};
	}
}
