#pragma once
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

namespace Rendering {
	/** Frame Buffering levels, which determine the number of resources we'll cycle through for each frame */
	enum class EBuffering : uint8_t {
		None,
		Double,
		Triple,
	};

	enum class EPreparationResult : uint8_t {
		/** The next step can be taken for rendering */
		Success,
		/** There has been an error, but we can retry rendering next frame */
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
		VkSemaphore imageAvailableSemaphore = nullptr;
		VkSemaphore renderFinishedSemaphore = nullptr;
		VkFence fence = nullptr;

		static bool IsValid(FrameResources const& r) { return r.commandPool && r.commands && r.uniforms.global && r.uniforms.object && r.imageAvailableSemaphore && r.renderFinishedSemaphore && r.fence; }
	};

	/** Keeps track of the resources used each frame, and how they should be used. */
	struct VulkanFrameOrganizer {
		/** The resources that we will cycle through when rendering each frame */
		std::vector<FrameResources> resources;
		std::vector<VkFence> imageFences;

		/** Command recording objects */
		VkCommandPool commandPool = nullptr;
		VkCommandBuffer stagingCommandBuffer = nullptr;

		struct {
			VkDescriptorSetLayout global;
			VkDescriptorSetLayout object;
		} layout;

		/** The pool for descriptors used in each frame */
		VkDescriptorPool descriptorPool;

		size_t currentResourceIndex = 0;
		uint32_t currentImageIndex = -1;

		inline operator bool() const { return resources.size() > 0 && imageFences.size() > 0 && std::all_of(resources.begin(), resources.end(), FrameResources::IsValid); }

		bool Create(CTX_ARG, VulkanPhysicalDevice const& physical, VulkanLogicalDevice const& logical, EBuffering buffering, size_t numImages, size_t numThreads);
		void Destroy(VulkanLogicalDevice const& logical);

		/** Prepare the next set of resources for rendering */
		EPreparationResult Prepare(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, size_t numObjects);
		/** Submit everything currently recorded so that it can be rendered. */
		bool Submit(CTX_ARG, VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain);

		/** Record commands to the current buffer. Recorder must have the signature void(VkCommandBuffer,size_t). */
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
