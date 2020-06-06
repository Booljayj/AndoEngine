#include <cassert>
#include <glm/vec3.hpp>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include "Rendering/RenderingSystem.h"
#include "Engine/BasicComponents.h"
#include "Engine/LogCommands.h"
#include "Engine/Utility.h"
#include "EntityFramework/EntityCollectionSystem.h"
#include "Rendering/SDLSystems.h"
#include "Rendering/MeshRendererComponent.h"

RenderingSystem::RenderingSystem()
: shouldRecreateSwapchain(false)
{}

bool RenderingSystem::Startup(
	CTX_ARG,
	SDLWindowSystem* windowSystem,
	EntityCollectionSystem* entityCollectionSystem,
	TComponentInfo<TransformComponent>* transform,
	TComponentInfo<MeshRendererComponent>* meshRenderer)
{
	ComponentInfo const* infos[FilterSize] = { transform, meshRenderer };
	filter = entityCollectionSystem->MakeFilter(infos);
	if (filter) {
		transformHandle = filter->GetMatchComponentHandle(transform);
		meshRendererHandle = filter->GetMatchComponentHandle(meshRenderer);
		//return true;
	} else {
		//return false;
	}

	// Vulkan instance
	if (!framework.Create(CTX, windowSystem->GetMainWindow())) {
		LOG(Temp, Error, "Failed to create the Vulkan application");
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
			LOG(Temp, Error, "Failed to find any vulkan physical devices");
			return false;
		}

		if (!SelectPhysicalDevice(CTX, 0)) {
			LOG(Temp, Error, "Failed to select default physical device");
			return false;
		}
	}

	//Create the swapchain
	{
		shouldRecreateSwapchain = false;
		if (!swapchain.Create(CTX, VkExtent2D{1024, 768}, framework.surface, *GetPhysicalDevice(selectedPhysicalDeviceIndex), logicalDevice.device)) {
			LOG(Temp, Error, "Failed to create the swapchain");
			return false;
		}
	}

	return true;
}

bool RenderingSystem::Shutdown(CTX_ARG) {
	swapchain.Destroy(logicalDevice.device);
	logicalDevice.Destroy();
	framework.Destroy();
	return true;
}

bool RenderingSystem::SelectPhysicalDevice(CTX_ARG, uint32_t index) {
	if (index != selectedPhysicalDeviceIndex) {
		if (const Rendering::VulkanPhysicalDevice* physicalDevice = GetPhysicalDevice(index)) {
			TArrayView<char const*> const extensionNames = Rendering::VulkanPhysicalDevice::GetExtensionNames(CTX);

			logicalDevice.Destroy();
			if (!logicalDevice.Create(CTX, *physicalDevice, enabledFeatures, extensionNames)) {
				LOGF(Temp, Error, "Failed to create logical device for physical device %i", index);
				return false;
			}

			selectedPhysicalDeviceIndex = index;
			shouldRecreateSwapchain = true;

			physicalDevice->WriteDescription(std::cout);
			return true;
		}
	}
	return false;
}

void RenderingSystem::RenderFrame(float interpolationAlpha) const {
	for (EntityFilter<FilterSize>::FilterMatch const& match : *filter) {
		RenderComponent(match.Get(meshRendererHandle));
	}
}

void RenderingSystem::RenderComponent(MeshRendererComponent const* meshRenderer) {
	if (!meshRenderer->IsValid()) return;

	//glBindVertexArray(meshRenderer->VertexArrayID);
	//glDrawArrays(GL_TRIANGLES, 0, meshRenderer->VertexCount);
	//GLenum ErrorCode = glGetError();
	//if (ErrorCode != GL_NO_ERROR) {
	//	std::cerr << "OpenGL Error: " << ErrorCode << std::endl;
	//}
}

bool RenderingSystem::IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames) {
	return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(extensionNames) && physicalDevice.HasSwapchainSupport();
}
