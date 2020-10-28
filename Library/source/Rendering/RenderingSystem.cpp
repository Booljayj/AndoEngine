#include <SDL2/SDL_vulkan.h>
#include "Rendering/RenderingSystem.h"
#include "Engine/LogCommands.h"
#include "Engine/STL.h"
#include "Engine/Utility.h"
#include "Geometry/LinearAlgebra.h"
#include "Rendering/SDLSystems.h"

DEFINE_LOG_CATEGORY(Rendering, Warning);

RenderingSystem::RenderingSystem()
: shouldRecreateSwapchain(false)
{}

bool RenderingSystem::Startup(CTX_ARG, SDLWindowSystem& windowSystem, EntityRegistry& registry) {
	// Vulkan instance
	if (!framework.Create(CTX, windowSystem.GetMainWindow())) {
		LOG(Rendering, Error, "Failed to create the Vulkan framework");
	}

	// Collect physical devices and select default one
	{
		TArrayView<char const*> const extensionNames = Rendering::VulkanPhysicalDevice::GetExtensionNames(CTX);

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(framework.instance, &deviceCount, nullptr);
		VkPhysicalDevice* devices = CTX.temp.Request<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(framework.instance, &deviceCount, devices);

		for (int32_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
			const Rendering::VulkanPhysicalDevice physicalDevice = Rendering::VulkanPhysicalDevice::Get(CTX, devices[deviceIndex], framework.surface);
			if (IsUsablePhysicalDevice(physicalDevice, extensionNames)) {
				availablePhysicalDevices.push_back(physicalDevice);
			}
		}

		if (availablePhysicalDevices.size() == 0) {
			LOG(Rendering, Error, "Failed to find any vulkan physical devices");
			return false;
		}

		if (!SelectPhysicalDevice(CTX, 0)) {
			LOG(Rendering, Error, "Failed to select default physical device");
			return false;
		}
	}

	//Create the initial swapchain
	{
		shouldRecreateSwapchain = false;
		swapchain = Rendering::VulkanSwapchain::Create(CTX, VkExtent2D{1024, 768}, framework.surface, *selectedPhysicalDevice, logicalDevice);
		if (!swapchain) {
			LOG(Rendering, Error, "Failed to create the swapchain");
			return false;
		}
	}

	return true;
}

bool RenderingSystem::Shutdown(CTX_ARG) {
	swapchain.Destroy(logicalDevice);
	logicalDevice.Destroy();
	framework.Destroy();
	return true;
}

bool RenderingSystem::Update(CTX_ARG, Time time) {
	//Recreate the swapchain if necessary. This happens periodically if the rendering parameters have changed significantly.
	if (shouldRecreateSwapchain) {
		shouldRecreateSwapchain = false;

		LOG(Rendering, Info, "Recreating swapchain");
		swapchain = Rendering::VulkanSwapchain::Recreate(CTX, swapchain, VkExtent2D{1024, 768}, framework.surface, *selectedPhysicalDevice, logicalDevice);
		if (!swapchain) {
			LOG(Rendering, Error, "Failed to create a new swapchain");
			return true;
		}
	}

	//@todo perform actual rendering here
	return false;
}

bool RenderingSystem::SelectPhysicalDevice(CTX_ARG, uint32_t index) {
	using namespace Rendering;

	if (index != selectedPhysicalDeviceIndex) {
		if (VulkanPhysicalDevice const* physicalDevice = GetPhysicalDevice(index)) {
			TArrayView<char const*> const extensions = VulkanPhysicalDevice::GetExtensionNames(CTX);

			VulkanLogicalDevice newLogicalDevice = VulkanLogicalDevice::Create(CTX, *physicalDevice, features, extensions);
			if (!newLogicalDevice) {
				LOGF(Rendering, Error, "Failed to create logical device for physical device %i", index);
				return false;
			}
			logicalDevice = std::move(newLogicalDevice);

			selectedPhysicalDevice = physicalDevice;
			selectedPhysicalDeviceIndex = index;
			shouldRecreateSwapchain = true;

			physicalDevice->WriteDescription(std::cout);
			return true;
		}
	}
	return false;
}

bool RenderingSystem::IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames) {
	return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(extensionNames) && physicalDevice.HasSwapchainSupport();
}
