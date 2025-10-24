#include "Rendering/Surface.h"
#include "Rendering/Vulkan/QueueSelection.h"

namespace Rendering {
	SurfaceFrameOrganizer::SurfaceFrameOrganizer(VkDevice device, VmaAllocator allocator, SurfaceQueues const& queues, Swapchain const& swapchain, UniformLayouts const& uniform_layouts, EBuffering buffering)
		: device(device)
		, allocator(allocator)
		, global_layout(uniform_layouts.global)
		, object_layout(uniform_layouts.object)
		, swapchain(swapchain)
		, queues(queues)
	{
		size_t const num_frames = GetNumFrames(buffering);
		size_t const num_images = swapchain.GetNumImages();

		//Create the resources that are unique to each frame
		for (size_t index = 0; index < num_frames; ++index) {
			frames.emplace_back(device, queues.graphics, global_layout, object_layout, allocator);
		}

		//Create the resources that are unique to each swapchain image
		imageFences.resize(num_images, nullptr);

		imageSubmittedSemaphores.reserve(num_images);
		for (size_t index = 0; index < num_images; ++index) {
			imageSubmittedSemaphores.emplace_back(device, 0);
		}
	}

	FrameResources* SurfaceFrameOrganizer::PrepareFrame(std::span<ViewRenderingParameters> view_params, ResourcesCollection& previous_resources) {
		//The timeout duration in nanoseconds to wait for the frame resources to finish the previous render.
		//If the previous render takes longer than this, then this render call has failed.
		constexpr auto timeout = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(5));

		FrameResources& frame = frames[currentFrameIndex];

		//Wait until the frame is no longer being used. If this takes too long, we'll have to skip rendering.
		if (!frame.fence.WaitUntilSignalled(timeout)) {
			LOG(Vulkan, Warning, "Timed out waiting for fence");
			return nullptr;
		}

		frame.Prepare(device, view_params, queues.graphics, global_layout, object_layout, allocator, previous_resources);

		//Acquire the swapchain image that can be used for this frame
		frame.current_image_index = 0;
		if (vkAcquireNextImageKHR(device, swapchain, timeout.count(), frame.image_available_semaphore, VK_NULL_HANDLE, &frame.current_image_index) != VK_SUCCESS) {
			LOG(Vulkan, Warning, "Timed out waiting for next available swapchain image");
			return nullptr;
		}

		if (frame.current_image_index >= imageFences.size()) {
			throw FormatType<std::runtime_error>("AcquireNextImage index is out of range: %i >= %i", frame.current_image_index, imageFences.size());
		}

		//If another frame is still using this image, wait for it to complete
		if (imageFences[frame.current_image_index] != VK_NULL_HANDLE) {
			if (vkWaitForFences(device, 1, &imageFences[frame.current_image_index], VK_TRUE, timeout.count()) != VK_SUCCESS) {
				LOG(Vulkan, Warning, "Timed out waiting for image fence {}", frame.current_image_index);
				return nullptr;
			}
		}

		//This frame will now be associated with a new image, so stop tracking this frame's fence with any other image
		ranges::replace(imageFences, (VkFence)frame.fence, VkFence{ VK_NULL_HANDLE });

		return &frame;
	}

	void SurfaceFrameOrganizer::Submit(FrameResources const& frame) {
		for (ViewResources const& view : frame.views) {
			view.uniforms.global.Flush();
			view.uniforms.object.Flush();
		}

		//Assign this frame's fence as the one using the swap image so other frames that try to use this image know to wait
		imageFences[frame.current_image_index] = frame.fence;

		//Reset the fence for this frame, so we can start another rendering process that it will track
		frame.fence.Reset();

		//Set up information for how commands should be processed when submitted to the queue
		VkSemaphore const imageAvailableSemaphores[] = { frame.image_available_semaphore };
		VkPipelineStageFlags const imageAVailableStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		static_assert(std::size(imageAvailableSemaphores) == std::size(imageAVailableStages), "Number of semaphores must equal number of stages");

		VkSemaphore const submitFinishedSemaphores[] = { imageSubmittedSemaphores[frame.current_image_index] };

		queues.graphics.Submit(imageAvailableSemaphores, imageAVailableStages, frame.view_command_buffers, submitFinishedSemaphores, frame.fence);
		queues.present.Present(submitFinishedSemaphores, MakeSpan(swapchain), MakeSpan(frame.current_image_index));

		//We've successfully finished rendering this frame, so move to the next frame
		currentFrameIndex = (currentFrameIndex + 1) % frames.size();
	}

	SurfaceFrameOrganizer::PoolSizesType SurfaceFrameOrganizer::GetPoolSizes(EBuffering buffering) {
		uint32_t const num_frames = static_cast<uint32_t>(GetNumFrames(buffering));

		//Each frame should have:
		//- One VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER (used by global uniforms)
		//- One VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC (used by object uniforms)
		//- Up to 16 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER (used by samplers in the global or object uniforms)
		//- Two descriptor sets (for global and object uniforms)
		return {
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, num_frames },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, num_frames },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, num_frames * 16 },
		};
	}

	Surface::Surface(VkInstance instance, HAL::Window& inWindow)
		: instance(instance)
		, window(inWindow)
		, retryCount(0)
		, shouldRecreateSwapchain(false)
	{
		if (SDL_Vulkan_CreateSurface(window, instance, &surface) != SDL_TRUE || !surface) {
			throw std::runtime_error{ "Failed to create Vulkan window surface" };
		}

		glm::i32vec2 drawableSize = { 1, 1 };
		SDL_Vulkan_GetDrawableSize(window, &drawableSize.x, &drawableSize.y);
		windowSize = glm::u32vec2{ static_cast<uint32_t>(drawableSize.x), static_cast<uint32_t>(drawableSize.y) };
	}

	Surface::~Surface() {
		organizer.reset();
		framebuffers.reset();
		swapchain.reset();
		
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	glm::u32vec2 Surface::GetExtent() const {
		return swapchain->GetExtent();
	}

	void Surface::InitializeRendering(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& uniform_layouts) {
		if (queues || swapchain) throw std::runtime_error{ "Initializing rendering on a surface which is already initialized" };

		QueueFamilySelectors selectors{ physical.GetSurfaceFamilies(*this) };
		
		auto const references = selectors.SelectSurfaceQueues();
		if (!references) {
			LOG(Vulkan, Warning, "Physical device does not support required queues for surface {}. Cannot initialize rendering.", GetID());
			return;
		}

		queues = device.queues.Resolve(*references);
		if (!queues) {
			LOG(Vulkan, Warning, "Device does not contain required queues for surface {}. Cannot initialize rendering.", GetID());
			return;
		}

		PhysicalDevicePresentation const presentation = PhysicalDevicePresentation::GetPresentation(physical, *this).value();
		PhysicalDeviceCapabilities const capabilities{ physical, *this };

		swapchain.emplace(device, nullptr, presentation, capabilities, *this);
		framebuffers.emplace(device, *swapchain, passes);
		organizer.emplace(device, device, *queues, *swapchain, uniform_layouts, EBuffering::Double);
	}

	void Surface::DeinitializeRendering() {
		organizer.reset();
		framebuffers.reset();
		swapchain.reset();
		queues.reset();
	}

	bool Surface::RecreateSwapchain(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& uniform_layouts) {
		organizer.reset();
		framebuffers.reset();

		PhysicalDevicePresentation const presentation = PhysicalDevicePresentation::GetPresentation(physical, *this).value();
		PhysicalDeviceCapabilities const capabilities{ physical, *this };

		if (swapchain.has_value()) {
			//The previous swapchain needs to exist long enough to create the new one, so we create a temporary new value before assigning it.
			auto recreated = std::make_optional<Swapchain>(device, &swapchain.value(), presentation, capabilities, *this);
			swapchain.swap(recreated);
		} else {
			swapchain.emplace(device, nullptr, presentation, capabilities, *this);
		}

		framebuffers.emplace(device, *swapchain, passes);
		organizer.emplace(device, device, *queues, *swapchain, uniform_layouts, EBuffering::Double);

		return true;
	}

	Framebuffer const& Surface::GetFramebuffer(uint32_t image_index) const {
		return framebuffers->surface[image_index];
	}

	FrameResources* Surface::PrepareFrame(std::span<ViewRenderingParameters> view_params, ResourcesCollection& previous_resources) {
		return organizer->PrepareFrame(view_params, previous_resources);
	}

	void Surface::SubmitFrame(FrameResources& frame) {
		organizer->Submit(frame);
	}
}
