#pragma once
#include "Engine/Context.h"
#include "Engine/Logging/Logger.h"
#include "EntityFramework/EntityRegistry.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanFramework.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

DECLARE_LOG_CATEGORY(Rendering);

struct SDLWindowSystem;

class RenderingSystem {
private:
	/** The entity group that contains all entities to render */
	//EntityGroup<TypeList<MeshRendererComponent>, TypeList<TransformComponent>, TypeList<>> RenderableEntities;

	/** The vulkan framework for this application */
	Rendering::VulkanFramework framework;

	/** The enabled features on any physical device that this application uses */
	VkPhysicalDeviceFeatures enabledFeatures;

	/** The available physical devices which can be used */
	std::vector<Rendering::VulkanPhysicalDevice> availablePhysicalDevices;
	/** The index of the currently selected device */
	uint32_t selectedPhysicalDeviceIndex = (uint32_t)-1;

	/** The logical device for the currently selected physical device */
	Rendering::VulkanLogicalDevice logicalDevice;

	/** The swapchain that is currently being used for images */
	Rendering::VulkanSwapchain swapchain;

	/** Flags for tracking rendering behavior */
	uint32_t shouldRecreateSwapchain : 1;

public:
	RenderingSystem();

	bool Startup(
		CTX_ARG,
		SDLWindowSystem& windowSystem,
		EntityRegistry& registry
	);
	bool Shutdown(CTX_ARG);

	inline uint32_t NumPhysicalDevices() const { return availablePhysicalDevices.size(); }
	inline const Rendering::VulkanPhysicalDevice* GetPhysicalDevice(uint32_t Index) const {
		if (Index < NumPhysicalDevices()) return &availablePhysicalDevices[Index];
		else return nullptr;
	}
	bool SelectPhysicalDevice(CTX_ARG, uint32_t Index);

private:
	bool IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames);
};
