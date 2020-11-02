#pragma once
#include "Engine/Context.h"
#include "Engine/Logging/Logger.h"
#include "Engine/Time.h"
#include "EntityFramework/EntityRegistry.h"
#include "Rendering/Vulkan/VulkanCommands.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanFramework.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanSynchronizer.h"

struct SDLWindowSystem;

DECLARE_LOG_CATEGORY(Rendering);

class RenderingSystem {
public:
	/** The vulkan framework for this application */
	Rendering::VulkanFramework framework;

	/** The enabled features on any physical device that this application uses */
	VkPhysicalDeviceFeatures features;

	/** The available physical devices which can be used */
	std::vector<Rendering::VulkanPhysicalDevice> availablePhysicalDevices;
	/** The current selected physical device */
	Rendering::VulkanPhysicalDevice const* selectedPhysicalDevice = nullptr;
	/** The index of the currently selected physical device */
	uint32_t selectedPhysicalDeviceIndex = (uint32_t)-1;

	/** The logical device for the currently selected physical device */
	Rendering::VulkanLogicalDevice logicalDevice;

	/** The swapchain that is currently being used for images */
	Rendering::VulkanSwapchain swapchain;

	/** The commands that we can submit to the graphics device */
	Rendering::VulkanCommands commands;

	/** The synchronizer, which is used to coordinate rendering operations */
	Rendering::VulkanSynchronizer sync;

	/** Flags for tracking rendering behavior and changes */
	uint8_t shouldRecreateSwapchain : 1;
	uint8_t shouldRebuildCommandBuffers : 1;

	RenderingSystem();

	bool Startup(CTX_ARG, SDLWindowSystem& windowing, EntityRegistry& registry);
	bool Shutdown(CTX_ARG, EntityRegistry& registry);

	bool Update(CTX_ARG, Time const& time, EntityRegistry const& registry);

	inline uint32_t NumPhysicalDevices() const { return availablePhysicalDevices.size(); }
	inline const Rendering::VulkanPhysicalDevice* GetPhysicalDevice(uint32_t Index) const {
		if (Index < NumPhysicalDevices()) return &availablePhysicalDevices[Index];
		else return nullptr;
	}
	bool SelectPhysicalDevice(CTX_ARG, uint32_t Index);

private:
	bool IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames);
	bool RebuildCommandBuffer(CTX_ARG, EntityRegistry const& registry, VkCommandBuffer buffer, Rendering::VulkanSwapchainImageInfo info);
};
