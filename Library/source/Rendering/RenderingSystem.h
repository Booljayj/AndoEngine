#pragma once
#include "Engine/Context.h"
#include "Engine/Logging/Logger.h"
#include "Engine/Time.h"
#include "EntityFramework/EntityRegistry.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Vulkan/VulkanFramework.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

struct SDLWindowingSystem;

DECLARE_LOG_CATEGORY(Rendering);

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

	/** Flags for tracking rendering behavior and changes */
	uint8_t shouldRecreateSwapchain : 1;

	/** The number of consecutive times we have failed to render a frame */
	uint8_t retryCount = 0;

	RenderingSystem();

	bool Startup(CTX_ARG, SDLWindowingSystem& windowing, EntityRegistry& registry);
	bool Shutdown(CTX_ARG, EntityRegistry& registry);

	bool Render(CTX_ARG, EntityRegistry const& registry);

	inline uint32_t NumPhysicalDevices() const { return availablePhysicalDevices.size(); }
	inline const Rendering::VulkanPhysicalDevice* GetPhysicalDevice(uint32_t Index) const {
		if (Index < NumPhysicalDevices()) return &availablePhysicalDevices[Index];
		else return nullptr;
	}
	bool SelectPhysicalDevice(CTX_ARG, uint32_t Index);

protected:
	bool CreateRenderPasses(CTX_ARG);
	void DestroyRenderPasses(CTX_ARG);

private:
	bool IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames);
};
