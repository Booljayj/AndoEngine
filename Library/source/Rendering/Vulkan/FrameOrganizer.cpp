#include "Rendering/Vulkan/FrameOrganizer.h"
#include "Engine/TemporaryStrings.h"
#include "Rendering/UniformTypes.h"

namespace Rendering {
	FrameUniforms::FrameUniforms(VkDevice device, UniformLayouts const& uniformLayouts, VkDescriptorPool pool, VmaAllocator allocator)
		: sets(device, pool, { uniformLayouts.global, uniformLayouts.object })
		, global(device, sets[0], 1, allocator)
		, object(device, sets[1], 512, allocator)
	{}

	FrameSynchronization::FrameSynchronization(VkDevice inDevice)
		: imageAvailableSemaphore(inDevice, 0)
		, fence(inDevice)
	{}

	FrameResources::FrameResources(VkDevice device, GraphicsQueue graphics, UniformLayouts const& uniformLayouts, VkDescriptorPool descriptorPool, VmaAllocator allocator)
		: uniforms(device, uniformLayouts, descriptorPool, allocator)
		, mainCommandPool(device, graphics)
		, mainCommandBuffer(mainCommandPool.CreateBuffer(VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY))
		, sync(device)
	{}

	void FrameResources::Prepare(VkDevice device, size_t numObjects, size_t numThreads, GraphicsQueue graphics, ResourcesCollection& previous_resources) {
		//Assuming objects are evenly distributed across threads, this is the max number of objects that can be processed by each thread.
		const size_t max_objects_per_thread = (numObjects / numThreads) + 1;

		//Resize the object uniforms buffer so we can store all the per-object data that will be used for rendering, usually two sets of resources.
		uniforms.object.Reserve(numObjects);

		//Reset existing resources
		mainCommandPool.Reset();
		for (CommandPool& threadCommandPool : threadCommandPools) {
			threadCommandPool.Reset();
		}
		for (auto& resources : threadResources) {
			previous_resources << resources;
			resources.reserve(max_objects_per_thread);
		}

		//Grow the number of per-thread resources to match the number of threads
		for (size_t threadIndex = threadCommandPools.size(); threadIndex < numThreads; ++threadIndex) {
			CommandPool& newPool = threadCommandPools.emplace_back(device, graphics);
			threadCommandBuffers.emplace_back(newPool.CreateBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY));

			threadResources.emplace_back().reserve(max_objects_per_thread);
		}
	}

	FrameOrganizer::FrameOrganizer(VkDevice device, VmaAllocator allocator, SurfaceQueues const& queues, Swapchain const& swapchain, UniformLayouts const& uniformLayouts, EBuffering buffering)
		: device(device)
		, swapchain(swapchain)
		, queues(queues)
		, descriptorPool(device, GetPoolSizes(buffering), GetNumFrames(buffering) * 2)
	{
		size_t const numFrames = GetNumFrames(buffering);
		size_t const numImages = swapchain.GetNumImages();

		//Create the resources that are unique to each frame
		for (size_t index = 0; index < numFrames; ++index) {
			frames.emplace_back(device, queues.graphics, uniformLayouts, descriptorPool, allocator);
		}

		//Create the resources that are unique to each swapchain image
		imageFences.resize(numImages, nullptr);
		
		imageSubmittedSemaphores.reserve(numImages);
		for (size_t index = 0; index < numImages; ++index) {
			imageSubmittedSemaphores.emplace_back(device, 0);
		}
	}

	std::optional<RecordingContext> FrameOrganizer::CreateRecordingContext(size_t numObjects, size_t numThreads, ResourcesCollection& previous_resources) {
		//The timeout duration in nanoseconds to wait for the frame resources to finish the previous render.
		//If the previous render takes longer than this, then this render call has failed.
		constexpr auto timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(5));

		FrameResources& frame = frames[currentFrameIndex];

		//Wait until the frame is no longer being used. If this takes too long, we'll have to skip rendering.
		if (!frame.sync.fence.WaitUntilSignalled(timeout)) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return std::optional<RecordingContext>{};
		}

		frame.Prepare(device, numObjects, numThreads, queues.graphics, previous_resources);

		//Acquire the swapchain image that can be used for this frame
		currentImageIndex = 0;
		if (vkAcquireNextImageKHR(device, swapchain, timeout.count(), frame.sync.imageAvailableSemaphore, VK_NULL_HANDLE, &currentImageIndex) != VK_SUCCESS) {
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

		//This frame will now be associated with a new image, so stop tracking this frame's fence with any other image
		ranges::replace(imageFences, (VkFence)frame.sync.fence, VkFence{ VK_NULL_HANDLE });

		return RecordingContext{ currentFrameIndex, currentImageIndex, frame.uniforms, frame.mainCommandBuffer, frame.threadCommandBuffers, frame.threadResources };
	}

	void FrameOrganizer::Submit(std::span<VkCommandBuffer const> commands) {
		FrameResources const& frame = frames[currentFrameIndex];

		frame.uniforms.global.Flush();
		frame.uniforms.object.Flush();
		
		//Assign this frame's fence as the one using the swap image so other frames that try to use this image know to wait
		imageFences[currentImageIndex] = frame.sync.fence;

		//Reset the fence for this frame, so we can start another rendering process that it will track
		frame.sync.fence.Reset();

		//Set up information for how commands should be processed when submitted to the queue
		VkSemaphore const submitWaitSemaphores[] = { frame.sync.imageAvailableSemaphore };
		VkSemaphore const submitFinishedSemaphores[] = { imageSubmittedSemaphores[currentImageIndex] };

		VkPipelineStageFlags const waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		static_assert(std::size(submitWaitSemaphores) == std::size(waitStages), "Number of semaphores must equal number of stages");

		//Submit the actual command buffers
		VkSubmitInfo const submitInfo{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = static_cast<uint32_t>(std::size(submitWaitSemaphores)),
			.pWaitSemaphores = submitWaitSemaphores,
			.pWaitDstStageMask = waitStages,

			.commandBufferCount = static_cast<uint32_t>(commands.size()),
			.pCommandBuffers = commands.data(),
			
			.signalSemaphoreCount = static_cast<uint32_t>(std::size(submitFinishedSemaphores)),
			.pSignalSemaphores = submitFinishedSemaphores,
		};
		queues.graphics.Submit(submitInfo, frame.sync.fence);

		//Set up information for how to present the rendered image once commands are finished on the queue
		VkPresentInfoKHR const presentInfo{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = submitFinishedSemaphores,

			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &currentImageIndex,

			.pResults = nullptr, //Optional
		};
		queues.present.Present(presentInfo);

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
}
