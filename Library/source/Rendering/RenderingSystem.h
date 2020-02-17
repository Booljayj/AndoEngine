#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include <ostream>
#include "Engine/Context.h"
#include "Engine/Logging/Logger.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/EntityFilter.h"
#include "Rendering/Vulkan/VulkanApplication.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

struct SDLWindowSystem;
struct TransformComponent;
struct MeshRendererComponent;
struct EntityCollectionSystem;

class RenderingSystem {
private:
	static constexpr size_t FilterSize = 2;
	std::shared_ptr<EntityFilter<FilterSize>> filter;
	TComponentHandle<TransformComponent> transformHandle;
	TComponentHandle<MeshRendererComponent> meshRendererHandle;

	/** The vulkan application instance for this application */
	Rendering::VulkanApplication application;

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
		SDLWindowSystem* windowSystem,
		EntityCollectionSystem* entityCollectionSystem,
		TComponentInfo<TransformComponent>* transform,
		TComponentInfo<MeshRendererComponent>* meshRenderer
	);
	bool Shutdown(CTX_ARG);

	inline uint32_t NumPhysicalDevices() const { return availablePhysicalDevices.size(); }
	inline const Rendering::VulkanPhysicalDevice* GetPhysicalDevice(uint32_t Index) const {
		if (Index < NumPhysicalDevices()) return &availablePhysicalDevices[Index];
		else return nullptr;
	}
	bool SelectPhysicalDevice(CTX_ARG, uint32_t Index);

	void RenderFrame(float InterpolationAlpha) const;
	static void RenderComponent(MeshRendererComponent const* MeshRenderer);

private:
	bool IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames);
};
