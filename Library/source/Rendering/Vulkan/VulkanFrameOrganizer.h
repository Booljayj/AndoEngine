#pragma once
#include "Engine/Logging.h"
#include "Rendering/Uniforms.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanCommandBuffers.h"
#include "Rendering/Vulkan/VulkanDescriptors.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanUniformLayouts.h"

namespace Rendering {
	/** Buffering levels, which determine the number of frames that will be cycled through for rendering */
	enum class EBuffering : uint8_t {
		None,
		Double,
		Triple,
	};
	inline size_t GetNumFrames(EBuffering buffering) { return static_cast<size_t>(buffering) + 1; };

	/** The result of preparing to render a frame */
	enum class EPreparationResult : uint8_t {
		/** The next step can be taken for rendering */
		Success,
		/** There has been an error that prevents rendering, but we can retry rendering next frame */
		Retry,
		/** There has been a serious problem, and we should shut down */
		Error,
	};

	/** Uniforms used for each frame */
	struct FrameUniforms {
		using GlobalUniformsType = Uniforms<GlobalUniforms, EUniformsIndexing::NonDynamic>;
		using ObjectUniformsType = Uniforms<ObjectUniforms, EUniformsIndexing::Dynamic>;

		DescriptorSets<2> sets;
		GlobalUniformsType global;
		ObjectUniformsType object;

		FrameUniforms(VkDevice inDevice, VulkanUniformLayouts const& uniformLayouts, VkDescriptorPool pool, VmaAllocator allocator);
		FrameUniforms(FrameUniforms const&) = delete;
		FrameUniforms(FrameUniforms&&) noexcept;
	};

	/** Synchronization objects used each frame to coordinate rendering operations */
	struct FrameSynchronization {
	public:
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

		FrameResources(VkDevice inDevice, uint32_t graphicsQueueFamilyIndex, VulkanUniformLayouts const& uniformLayouts, VkDescriptorPool descriptorPool, VmaAllocator allocator);
		FrameResources(FrameResources const&) = delete;
		FrameResources(FrameResources&&) noexcept;
	};

	struct RecordingContext {
		size_t imageIndex;
		size_t frameIndex;
		FrameUniforms& uniforms;
		VkCommandBuffer mainCommandBuffer;
		TArrayView<VkCommandBuffer> threadCommandBuffers;
	};

	/** Keeps track of the resources used each frame, and how they should be used to render a number of viewports. */
	struct VulkanFrameOrganizer {
		VulkanFrameOrganizer(VulkanLogicalDevice const& logical, VulkanPhysicalDevice const& physical, VulkanSwapchain const& swapchain, VulkanUniformLayouts const& uniformLayouts, EBuffering buffering);
		VulkanFrameOrganizer(VulkanFrameOrganizer const&) = delete;
		VulkanFrameOrganizer(VulkanFrameOrganizer&&) noexcept;

		/**
		 * Create a recording context for the current frame using an unused set of resources.
		 * Returngs nothing if all resources are in use and we should skip recording this frame.
		 */
		std::optional<RecordingContext> CreateRecordingContext(size_t numObjects, size_t numThreads);

		/** Submit everything currently recorded so that it can be rendered. */
		void Submit(TArrayView<VkCommandBuffer const> commands);

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
		std::vector<FrameResources> resources;
		std::vector<VkFence> imageFences;

		size_t currentResourceIndex = 0;
		uint32_t currentImageIndex = -1;

		/** Get the pool sizes that should be used with the type of buffering */
		static PoolSizesType GetPoolSizes(EBuffering buffering);
	};
}
