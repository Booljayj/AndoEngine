#pragma once
#include "Engine/Logging.h"
#include "Engine/StandardTypes.h"
#include "Rendering/UniformTypes.h"
#include "Rendering/Vulkan/Commands.h"
#include "Rendering/Vulkan/Descriptors.h"
#include "Rendering/Vulkan/Device.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Vulkan/UniformLayouts.h"
#include "Rendering/Vulkan/Uniforms.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Buffering levels, which determine the number of frames that will be cycled through for rendering */
	enum class EBuffering : uint8_t {
		None,
		Double,
		Triple,
	};
	inline size_t GetNumFrames(EBuffering buffering) { return static_cast<size_t>(buffering) + 1; };

	/** Uniforms used for each frame */
	struct FrameUniforms {
		using GlobalUniformsType = Uniforms<GlobalUniforms>;
		using ObjectUniformsType = Uniforms<ObjectUniforms>;

		DescriptorSets<2> sets;
		GlobalUniformsType global;
		ObjectUniformsType object;

		FrameUniforms(VkDevice inDevice, UniformLayouts const& uniformLayouts, VkDescriptorPool pool, VmaAllocator allocator);
		FrameUniforms(FrameUniforms const&) = delete;
		FrameUniforms(FrameUniforms&&) noexcept;
	};

	/** Synchronization objects used each frame to coordinate rendering operations */
	struct FrameSynchronization {
		VkSemaphore imageAvailable = nullptr;
		VkSemaphore renderFinished = nullptr;
		VkFence fence = nullptr;
		
		FrameSynchronization(VkDevice inDevice);
		FrameSynchronization(FrameSynchronization const&) = delete;
		FrameSynchronization(FrameSynchronization&&) noexcept;
		~FrameSynchronization();

	private:
		VkDevice device = nullptr;
	};

	/** Resources used for a single frame of rendering */
	struct FrameResources {
		FrameUniforms uniforms;

		CommandPool mainCommandPool;
		VkCommandBuffer mainCommandBuffer = nullptr;

		std::vector<CommandPool> threadCommandPools;
		std::vector<VkCommandBuffer> threadCommandBuffers;

		FrameSynchronization sync;
		RenderKey key;

		FrameResources(VkDevice inDevice, uint32_t graphicsQueueFamilyIndex, UniformLayouts const& uniformLayouts, VkDescriptorPool descriptorPool, VmaAllocator allocator);
		FrameResources(FrameResources const&) = delete;
		FrameResources(FrameResources&&) noexcept;
	};

	struct RecordingContext {
		/** The index of the buffering frame */
		uint32_t frameIndex;
		/** The index of the image in the swapchain */
		uint32_t imageIndex;
		/** The unique key for the current render operation. Used to mark dependent resources so we know when they were last used (or are still in use). */
		RenderKey key;

		/** Common uniforms used when recording */
		FrameUniforms& uniforms;
		/** Command buffers used to record commands */
		VkCommandBuffer primaryCommandBuffer;
		std::span<VkCommandBuffer> secondaryCommandBuffers;
	};

	/** Keeps track of the resources used each frame, and how they should be used to render a number of viewports. */
	struct FrameOrganizer {
		FrameOrganizer(VkDevice device, VmaAllocator allocator, SurfaceQueues const& queues, Swapchain const& swapchain, UniformLayouts const& uniformLayouts, EBuffering buffering);
		FrameOrganizer(FrameOrganizer const&) = delete;
		FrameOrganizer(FrameOrganizer&&) noexcept;

		/**
		 * Create a recording context for the current frame using an unused set of resources.
		 * Returngs nothing if all resources are in use and we should skip recording this frame.
		 */
		std::optional<RecordingContext> CreateRecordingContext(RenderKey key, size_t numObjects, size_t numThreads);

		/** Submit everything currently recorded so that it can be rendered. */
		void Submit(std::span<VkCommandBuffer const> commands);

		t_vector<RenderKey> GetInProgressRenderKeys() const;

	private:
		using PoolSizesType = std::array<VkDescriptorPoolSize, 3>;

		VkDevice device = nullptr;
		VkSwapchainKHR swapchain = nullptr;
		struct {
			VkQueue graphics = nullptr;
			VkQueue present = nullptr;
		} queue;
		uint32_t graphicsQueueFamilyIndex = 0;

		DescriptorPool descriptorPool;

		/** The frames that we will cycle through when rendering */
		std::vector<FrameResources> frames;
		/** Copies of the per-frame resources that are being used for each swapchain image */
		std::vector<VkFence> imageFences;

		uint32_t currentFrameIndex = 0;
		uint32_t currentImageIndex = -1;

		/** Get the pool sizes that should be used with the type of buffering */
		static PoolSizesType GetPoolSizes(EBuffering buffering);
	};
}
