#pragma once
#include "Engine/Core.h"
#include "Engine/GLM.h"
#include "Engine/Optional.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/RenderTarget.h"
#include "Rendering/Vulkan/RenderPasses.h"
#include "Rendering/Vulkan/Semaphore.h"
#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct PhysicalDeviceDescription;
	struct RenderingSystem;
	struct ResourcesCollection;
	struct UniformLayouts;

	/** Buffering levels, which determine the number of frames that will be cycled through for rendering */
	enum class EBuffering : uint8_t {
		None,
		Double,
		Triple,
	};
	inline size_t GetNumFrames(EBuffering buffering) { return static_cast<size_t>(buffering) + 1; };

	/** Keeps track of the resources used each frame for a surface */
	struct SurfaceFrameOrganizer {
		SurfaceFrameOrganizer(VkDevice device, VmaAllocator allocator, SurfaceQueues const& queues, Swapchain const& swapchain, UniformLayouts const& uniform_layouts, EBuffering buffering);
		SurfaceFrameOrganizer(SurfaceFrameOrganizer const&) = delete;
		SurfaceFrameOrganizer(SurfaceFrameOrganizer&&) noexcept = default;

		/**
		 * Prepare the resources for the next frame for rendering. This will wait for resources to be ready if they're not ready immediately.
		 * If the resources take too long to become ready, this will return nothing and no rendering should happen.
		 */
		FrameContext* GetNextFrameContext();

		/** Submit everything currently recorded so that it can be rendered. */
		void SubmitFrameContext(FrameContext const& frame);

	private:
		using PoolSizesType = std::array<VkDescriptorPoolSize, 3>;

		MoveOnly<VkDevice> device;
		VkDescriptorSetLayout global_layout = nullptr;
		VkDescriptorSetLayout object_layout = nullptr;
		VmaAllocator allocator = nullptr;
		VkSwapchainKHR swapchain = nullptr;
		SurfaceQueues queues;

		/** The frames that we will cycle through when rendering */
		std::vector<FrameContext> frames;
		/** Copies of the per-frame resources that are being used for each swapchain image */
		std::vector<VkFence> imageFences;
		/** Semaphores used to determine when the queue is finished using each swapchain image */
		std::vector<Semaphore> imageSubmittedSemaphores;

		uint32_t prevousFrameIndex = std::numeric_limits<uint32_t>::max();
		uint32_t currentFrameIndex = 0;

		/** Get the pool sizes that should be used with the type of buffering */
		static PoolSizesType GetPoolSizes(EBuffering buffering);
	};

	/** A surface tied to a given window, which is used by a RenderSystem for rendering */
	struct Surface : public RenderTarget {
	public:
		Surface(VkInstance instance, HAL::Window& inWindow);
		Surface(Surface const&) = delete;
		Surface(Surface&&) = delete;
		~Surface();

		operator VkSurfaceKHR() const { return surface; }
		inline bool operator==(HAL::Window::IdType otherID) const { return GetID() == otherID; }

		/** Get the id of the window associated with this surface */
		inline HAL::Window::IdType GetID() const { return window.id; }

		/** Return whether this is a valid surface that is prepared for rendering */
		inline bool CanRender() const { return surface && queues && swapchain; }
		/** Whether the swapchain needs to be recreated before it is used again */
		inline bool IsSwapchainDirty() const { return shouldRecreateSwapchain; }

		/** Create resources used for rendering */
		void InitializeRendering(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& uniform_layouts);
		/** Remove resources used for rendering */
		void DeinitializeRendering();

		/** Create or recreate the swapchain and related resources for this surface */
		bool RecreateSwapchain(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& uniform_layouts);
		
	protected:
		Framebuffer const& GetFramebuffer(uint32_t image_index) const override;
		FrameContext* GetNextFrameContext() override;
		void SubmitFrameContext(FrameContext const& frame) override;

	private:
		friend RenderingSystem;
		friend Swapchain;

		VkInstance instance = nullptr;
		HAL::Window& window;

		/** The internal surface tied to this surface */
		VkSurfaceKHR surface = nullptr;

		/** The queues that this surface will use to submit commands */
		std::optional<SurfaceQueues> queues;

		/** The swapchain that is currently being used for images */
		std::optional<Swapchain> swapchain;
		/** The framebuffers for rendering on the swapchain */
		std::optional<Framebuffers> framebuffers;
		/** The frame organizer that keeps track of resources used each frame */
		std::optional<SurfaceFrameOrganizer> organizer;

		/** The full size of the window that is associated with this surface */
		glm::u32vec2 windowSize;

		uint8_t retryCount : 1;
		uint8_t shouldRecreateSwapchain : 1;
	};
}
