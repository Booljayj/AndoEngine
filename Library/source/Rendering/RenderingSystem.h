#pragma once
#include "Engine/Context.h"
#include "Engine/Logging/Logger.h"
#include "Engine/Time.h"
#include "EntityFramework/EntityRegistry.h"
#include "Rendering/MaterialComponent.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Vulkan/VulkanFramework.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Rendering/Vulkan/VulkanShaderModuleLibrary.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

struct SDLWindowingSystem;

DECLARE_LOG_CATEGORY(Rendering);

namespace Rendering {
	class RenderingSystem {
	public:
		/** The maximum number of consecutive times we can fail to render a frame */
		static constexpr uint8_t maxRetryCount = 5;

		/** The enabled features on any physical device that this application uses */
		VkPhysicalDeviceFeatures features;

		/** The vulkan framework for this application */
		Rendering::VulkanFramework framework;

		/** The available physical devices which can be used */
		std::vector<Rendering::VulkanPhysicalDevice> availablePhysicalDevices;
		/** The current selected physical device */
		Rendering::VulkanPhysicalDevice const* selectedPhysical = nullptr;
		/** The index of the currently selected physical device */
		uint32_t selectedPhysicalIndex = (uint32_t)-1;

		/** The logical device for the currently selected physical device */
		Rendering::VulkanLogicalDevice logical;
		/** The swapchain that is currently being used for images */
		Rendering::VulkanSwapchain swapchain;
		/** The frame organizer that keeps track of resources used each frame */
		Rendering::VulkanFrameOrganizer organizer;

		/** The main render pass */
		Rendering::TRenderPassInfo<1> main;

		/** Bits for tracking rendering behavior and changes */
		uint8_t retryCount : 4;
		uint8_t shouldRecreateSwapchain : 1;
		uint8_t shouldCreatePipelines : 1;

		RenderingSystem();

		bool Startup(CTX_ARG, SDLWindowingSystem& windowing, EntityRegistry& registry);
		bool Shutdown(CTX_ARG, EntityRegistry& registry);

		bool Render(CTX_ARG, EntityRegistry& registry);

		inline uint32_t NumPhysicalDevices() const { return availablePhysicalDevices.size(); }
		inline const Rendering::VulkanPhysicalDevice* GetPhysicalDevice(uint32_t Index) const {
			if (Index < NumPhysicalDevices()) return &availablePhysicalDevices[Index];
			else return nullptr;
		}
		bool SelectPhysicalDevice(CTX_ARG, uint32_t Index);

	protected:
		/** Contains callbacks related to material component operations */
		struct MaterialComponentOperations {
			static void OnCreate(entt::registry& registry, entt::entity entity);
			static void OnDestroy(entt::registry& registry, entt::entity entity);
			static void OnModify(entt::registry& registry, entt::entity entity);
		};

		/** Pipeline resources that are pending destruction */
		std::vector<Rendering::VulkanPipelineResources> stalePipelineResources;

		/** Create or destroy the render passes used by this rendering system */
		bool CreateRenderPasses(CTX_ARG);
		void DestroyRenderPasses(CTX_ARG);

		/** Create or destroy all pipeline resources */
		void CreatePipelines(CTX_ARG, EntityRegistry& registry);
		void DestroyPipelines(EntityRegistry& registry);

		/** Create the pipeline resources for a material */
		void CreatePipeline(CTX_ARG, Rendering::MaterialComponent& material, EntityID id, Rendering::VulkanShaderModuleLibrary& library);
		/** Mark the pipeline resources on the material as stale */
		void MarkPipelineStale(Rendering::MaterialComponent& material);
		/** Destroy any stale pipeline resources */
		void DestroyStalePipelines();

	private:
		/** Returns true if the physical device can actually be used by this rendering system */
		bool IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames);
	};
}
