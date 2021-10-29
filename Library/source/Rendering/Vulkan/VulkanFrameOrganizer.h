#pragma once
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanUniformLayouts.h"

namespace Rendering {
	/** Buffering levels, which determine the number of frames we'll cycle through for rendering */
	enum class EBuffering : uint8_t {
		None,
		Double,
		Triple,
	};

	/** The result of preparing to render a frame */
	enum class EPreparationResult : uint8_t {
		/** The next step can be taken for rendering */
		Success,
		/** There has been an error that prevents rendering, but we can retry rendering next frame */
		Retry,
		/** There has been a serious problem, and we should shut down */
		Error,
	};

	/** Resources used for a single frame of rendering */
	struct FrameResources {
		/** Command recording objects */
		VkCommandPool commandPool = nullptr;
		VkCommandBuffer commands = nullptr;

		/** Uniforms */
		struct {
			VulkanUniforms global;
			VulkanUniforms object;
		} uniforms;

		/** Synchronization objects */
		struct {
			VkSemaphore imageAvailable = nullptr;
			VkSemaphore renderFinished = nullptr;
		} semaphores;
		VkFence fence = nullptr;

		static bool IsValid(FrameResources const& r) { return r.commandPool && r.commands && r.uniforms.global && r.uniforms.object && r.semaphores.imageAvailable && r.semaphores.renderFinished && r.fence; }
	};

	/** Keeps track of the resources used each frame, and how they should be used to render a number of viewports. */
	struct VulkanFrameOrganizer {
		/** The frames that we will cycle through when rendering */
		std::vector<FrameResources> resources;
		std::vector<VkFence> imageFences;

		/** Command recording objects */
		VkCommandPool commandPool = nullptr;
		VkCommandBuffer stagingCommandBuffer = nullptr;

		/** The pool for descriptors used in each frame */
		VkDescriptorPool descriptorPool = nullptr;

		size_t currentResourceIndex = 0;
		uint32_t currentImageIndex = -1;

		inline operator bool() const { return resources.size() > 0 && imageFences.size() > 0 && std::all_of(resources.begin(), resources.end(), FrameResources::IsValid); }

		bool Create(CTX_ARG, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical, VulkanUniformLayouts const& uniformLayouts, EBuffering buffering, size_t numImages, size_t numThreads);
		void Destroy(VulkanLogicalDevice const& logical);

		/** Prepare the next set of resources for rendering */
		EPreparationResult Prepare(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, size_t numObjects);
		/** Submit everything currently recorded so that it can be rendered. */
		bool Submit(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain);

		/** Record commands to the current buffer */
		template<typename FunctorType>
		inline bool Record(CTX_ARG, FunctorType&& recorder) {
			FrameResources const& frame = resources[currentResourceIndex];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(frame.commands, &beginInfo) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to begin recording command buffer");
				return false;
			}

			recorder(frame, currentImageIndex);

			if (vkEndCommandBuffer(frame.commands) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to finish recording command buffer");
				return false;
			}

			return true;
		}

		/** Wait for all work to complete, including all submitted rendering commands */
		void WaitForCompletion(CTX_ARG, VulkanLogicalDevice const& logical);
	};
}
