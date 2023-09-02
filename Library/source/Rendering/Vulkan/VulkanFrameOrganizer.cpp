#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/UniformTypes.h"

namespace Rendering {
	FrameUniforms::FrameUniforms(VkDevice inDevice, VulkanUniformLayouts const& uniformLayouts, VkDescriptorPool pool, VmaAllocator allocator)
		: sets(inDevice, pool, { uniformLayouts.global, uniformLayouts.object })
		, global(inDevice, sets[0], 1, allocator)
		, object(inDevice, sets[1], 512, allocator)
	{}

	FrameUniforms::FrameUniforms(FrameUniforms&& other) noexcept
		: sets(std::move(other.sets)), global(std::move(other.global)), object(std::move(other.object))
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

	FrameSynchronization::FrameSynchronization(FrameSynchronization&& other) noexcept
		: imageAvailable(other.imageAvailable), renderFinished(other.renderFinished), fence(other.fence), device(other.device)
	{
		other.device = nullptr;
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

	VulkanFrameOrganizer::VulkanFrameOrganizer(VulkanLogicalDevice const& logical, VulkanPhysicalDevice const& physical, Swapchain const& swapchain, VulkanUniformLayouts const& uniformLayouts, EBuffering buffering)
		: device(logical)
		, swapchain(swapchain)
		, queue({ logical.queues.graphics, logical.queues.present })
		, graphicsQueueFamilyIndex(physical.queues.graphics->index)
		, descriptorPool(device, GetPoolSizes(buffering), GetNumFrames(buffering) * 2)
	{
		size_t const numFrames = GetNumFrames(buffering);
		for (size_t index = 0; index < numFrames; ++index) {
			frames.emplace_back(device, graphicsQueueFamilyIndex, uniformLayouts, descriptorPool, logical.allocator);
		}
		imageFences.resize(swapchain.GetNumImages(), nullptr);
	}

	VulkanFrameOrganizer::VulkanFrameOrganizer(VulkanFrameOrganizer&& other) noexcept
		: device(other.device), swapchain(other.swapchain), queue({ other.queue.graphics, other.queue.present }), graphicsQueueFamilyIndex(other.graphicsQueueFamilyIndex)
		, descriptorPool(std::move(other.descriptorPool))
		, frames(std::move(other.frames)), imageFences(std::move(other.imageFences))
		, currentFrameIndex(other.currentFrameIndex), currentImageIndex(other.currentImageIndex)
	{
		other.device = nullptr;
	}

	std::optional<RecordingContext> VulkanFrameOrganizer::CreateRecordingContext(size_t numObjects, size_t numThreads) {
		//The timeout duration in nanoseconds
		constexpr uint64_t timeout = 5'000'000'000;

		FrameResources& frame = frames[currentFrameIndex];

		//Wait for the frame fence to finish, indicating this frame is no longer in use
		if (vkWaitForFences(device, 1, &frame.sync.fence, VK_TRUE, timeout) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return std::optional<RecordingContext>{};
		}

		//Reset the command pools so new commands can be written
		frame.mainCommandPool.Reset();
		for (CommandPool& threadCommandPool : frame.threadCommandPools) {
			threadCommandPool.Reset();
		}

		//Grow the number of command pools to match the number of threads
		for (size_t threadIndex = frame.threadCommandPools.size(); threadIndex < numThreads; ++threadIndex) {
			CommandPool& newPool = frame.threadCommandPools.emplace_back(device, graphicsQueueFamilyIndex);
			VkCommandBuffer& newBuffer = frame.threadCommandBuffers.emplace_back();

			newBuffer = newPool.CreateBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		}

		//Resize the object uniforms buffer so we can store all the per-object data that will be used for rendering
		frame.uniforms.object.Reserve(numObjects);

		//Acquire the swapchain image that can be used for this frame
		currentImageIndex = 0;
		if (vkAcquireNextImageKHR(device, swapchain, timeout, frame.sync.imageAvailable, VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for next available swapchain image");
			return std::optional<RecordingContext>{};
		}

		if (currentImageIndex >= imageFences.size()) {
			throw std::runtime_error{ t_printf("AcquireNextImage index is out of range: %i >= %i", currentImageIndex, imageFences.size()).data() };
		}

		//If another frame is still using this image, wait for it to complete
		if (imageFences[currentImageIndex] != VK_NULL_HANDLE) {
			if (vkWaitForFences(device, 1, &imageFences[currentImageIndex], VK_TRUE, timeout) != VK_SUCCESS) {
				LOGF(Vulkan, Warning, "Timed out waiting for image fence %i", currentImageIndex);
				return std::optional<RecordingContext>{};
			}
		}
		//This frame will now be assocaited with a new image, so stop tracking this frame's fence with any other image
		std::replace(imageFences.begin(), imageFences.end(), frame.sync.fence, VkFence{ VK_NULL_HANDLE });

		return RecordingContext{ currentFrameIndex, currentImageIndex, frame.uniforms, frame.mainCommandBuffer, frame.threadCommandBuffers };
	}

	void VulkanFrameOrganizer::Submit(TArrayView<VkCommandBuffer const> commands) {
		FrameResources const& frame = frames[currentFrameIndex];

		frame.uniforms.global.Flush();
		frame.uniforms.object.Flush();
		
		//Assign this frame's fence as the one using the swap image so other frames that try to use this image know to wait
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

		//We've successfully finished rendering this frame, so move to the next frame
		currentFrameIndex = (currentFrameIndex + 1) % frames.size();
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
