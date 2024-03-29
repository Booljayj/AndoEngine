#include "Rendering/Vulkan/FrameOrganizer.h"
#include "Rendering/Vulkan/Handles.h"
#include "Rendering/UniformTypes.h"

namespace Rendering {
	FrameUniforms::FrameUniforms(VkDevice device, UniformLayouts const& uniformLayouts, VkDescriptorPool pool, VmaAllocator allocator)
		: sets(device, pool, { uniformLayouts.global, uniformLayouts.object })
		, global(device, sets[0], 1, allocator)
		, object(device, sets[1], 512, allocator)
	{}

	FrameUniforms::FrameUniforms(FrameUniforms&& other) noexcept
		: sets(std::move(other.sets)), global(std::move(other.global)), object(std::move(other.object))
	{}

	FrameSynchronization::FrameSynchronization(VkDevice inDevice)
		: device(inDevice)
		, fence(inDevice)
	{
		//Create the semaphores
		VkSemaphoreCreateInfo const semaphoreInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		auto imageAvailableHandle = Semaphore::Create(device, semaphoreInfo, "Failed to create image available semaphore");
		auto renderFinishedHandle = Semaphore::Create(device, semaphoreInfo, "Failed to create render finished semaphore");

		imageAvailable = imageAvailableHandle.Release();
		renderFinished = renderFinishedHandle.Release();
	}

	FrameSynchronization::~FrameSynchronization() {
		if (device) {
			fence.WaitUntilSignalled(std::chrono::nanoseconds(std::numeric_limits<uint64_t>::max()));
			vkDestroySemaphore(device, renderFinished, nullptr);
			vkDestroySemaphore(device, imageAvailable, nullptr);
		}
	}

	FrameResources::FrameResources(VkDevice inDevice, uint32_t graphicsQueueFamilyIndex, UniformLayouts const& uniformLayouts, VkDescriptorPool descriptorPool, VmaAllocator allocator)
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

	bool FrameResources::WaitUntilUnused(std::chrono::nanoseconds timeout) const {
		return sync.fence.WaitUntilSignalled(timeout);
	}

	void FrameResources::Prepare(VkDevice device, size_t numObjects, size_t numThreads, uint32_t graphicsQueueFamilyIndex) {
		//Resize the object uniforms buffer so we can store all the per-object data that will be used for rendering
		uniforms.object.Reserve(numObjects);

		//Reset the command pools so new commands can be written
		mainCommandPool.Reset();
		for (CommandPool& threadCommandPool : threadCommandPools) {
			threadCommandPool.Reset();
		}

		//Grow the number of command pools to match the number of threads
		for (size_t threadIndex = threadCommandPools.size(); threadIndex < numThreads; ++threadIndex) {
			CommandPool& newPool = threadCommandPools.emplace_back(device, graphicsQueueFamilyIndex);
			threadCommandBuffers.emplace_back(newPool.CreateBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY));
		}

		//Release resources that were previously used to render this frame
		objects.clear();
	}

	FrameOrganizer::FrameOrganizer(VkDevice device, VmaAllocator allocator, SurfaceQueues const& queues, Swapchain const& swapchain, UniformLayouts const& uniformLayouts, EBuffering buffering)
		: device(device)
		, swapchain(swapchain)
		, queue({ queues.graphics, queues.present })
		, graphicsQueueFamilyIndex(queues.graphics.family)
		, descriptorPool(device, GetPoolSizes(buffering), GetNumFrames(buffering) * 2)
	{
		size_t const numFrames = GetNumFrames(buffering);
		for (size_t index = 0; index < numFrames; ++index) {
			frames.emplace_back(device, graphicsQueueFamilyIndex, uniformLayouts, descriptorPool, allocator);
		}
		imageFences.resize(swapchain.GetNumImages(), nullptr);
	}

	std::optional<RecordingContext> FrameOrganizer::CreateRecordingContext(size_t numObjects, size_t numThreads) {
		//The timeout duration in nanoseconds
		//constexpr uint64_t timeout = 5'000'000'000;
		constexpr auto timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(5));

		FrameResources& frame = frames[currentFrameIndex];

		//Wait until the frame is no longer being used. If this takes too long, we'll have to skip rendering.
		if (!frame.WaitUntilUnused(timeout)) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return std::optional<RecordingContext>{};
		}

		frame.Prepare(device, numObjects, numThreads, graphicsQueueFamilyIndex);

		//Acquire the swapchain image that can be used for this frame
		currentImageIndex = 0;
		if (vkAcquireNextImageKHR(device, swapchain, timeout.count(), frame.sync.imageAvailable, VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for next available swapchain image");
			return std::optional<RecordingContext>{};
		}

		if (currentImageIndex >= imageFences.size()) {
			throw FormatType<std::runtime_error>("AcquireNextImage index is out of range: %i >= %i", currentImageIndex, imageFences.size());
		}

		//If another frame is still using this image, wait for it to complete
		if (imageFences[currentImageIndex] != VK_NULL_HANDLE) {
			if (vkWaitForFences(device, 1, &imageFences[currentImageIndex], VK_TRUE, timeout.count()) != VK_SUCCESS) {
				LOG(Vulkan, Warning, "Timed out waiting for image fence {}", currentImageIndex);
				return std::optional<RecordingContext>{};
			}
		}
		//This frame will now be assocaited with a new image, so stop tracking this frame's fence with any other image
		std::replace(imageFences.begin(), imageFences.end(), (VkFence)frame.sync.fence, VkFence{ VK_NULL_HANDLE });

		return RecordingContext{ currentFrameIndex, currentImageIndex, frame.uniforms, frame.mainCommandBuffer, frame.threadCommandBuffers, frame.objects };
	}

	void FrameOrganizer::Submit(std::span<VkCommandBuffer const> commands) {
		FrameResources const& frame = frames[currentFrameIndex];

		frame.uniforms.global.Flush();
		frame.uniforms.object.Flush();
		
		//Assign this frame's fence as the one using the swap image so other frames that try to use this image know to wait
		imageFences[currentImageIndex] = frame.sync.fence;

		//Reset the fence for this frame, so we can start another rendering process that it will track
		frame.sync.fence.Reset();

		//Set up information for how commands should be process when submitted to the queue
		VkSemaphore const waitSemaphores[] = { frame.sync.imageAvailable };
		VkPipelineStageFlags const waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		static_assert(std::size(waitSemaphores) == std::size(waitStages), "Number of semaphores must equal number of stages");

		VkSemaphore const signalSemaphores[] = { frame.sync.renderFinished };

		VkSubmitInfo const submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = static_cast<uint32_t>(std::size(waitSemaphores)),
			.pWaitSemaphores = waitSemaphores,
			.pWaitDstStageMask = waitStages,

			.commandBufferCount = static_cast<uint32_t>(commands.size()),
			.pCommandBuffers = commands.data(),
			
			.signalSemaphoreCount = static_cast<uint32_t>(std::size(signalSemaphores)),
			.pSignalSemaphores = signalSemaphores,
		};

		//Submit the actual command buffer
		if (vkQueueSubmit(queue.graphics, 1, &submitInfo, frame.sync.fence) != VK_SUCCESS) {
			throw FormatType<std::runtime_error>("Failed to submit command buffer to qraphics queue");
		}

		//Set up information for how to present the rendered image once commands are finished on the queue
		VkSwapchainKHR const swapchains[] = { swapchain };
		
		VkPresentInfoKHR const presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = signalSemaphores,

			.swapchainCount = static_cast<uint32_t>(std::size(swapchains)),
			.pSwapchains = swapchains,
			.pImageIndices = &currentImageIndex,

			.pResults = nullptr, //Optional
		};
		
		if (vkQueuePresentKHR(queue.present, &presentInfo) != VK_SUCCESS) {
			throw FormatType<std::runtime_error>("Failed to present image");
		}

		//We've successfully finished rendering this frame, so move to the next frame
		currentFrameIndex = (currentFrameIndex + 1) % frames.size();
	}

	FrameOrganizer::PoolSizesType FrameOrganizer::GetPoolSizes(EBuffering buffering) {
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

	RenderObjectsHandleCollection& operator<<(RenderObjectsHandleCollection& collection, FrameResources& frame) {
		collection << std::move(frame.objects);
		return collection;
	}

	RenderObjectsHandleCollection& operator<<(RenderObjectsHandleCollection& collection, FrameOrganizer& organizer) {
		for (auto& frame : organizer.frames) collection << frame;
		return collection;
	}
}
