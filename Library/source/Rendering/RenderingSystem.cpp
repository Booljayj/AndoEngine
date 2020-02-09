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
	EntityCollectionSystem* EntityCollection,
	TComponentInfo<TransformComponent>* Transform,
	TComponentInfo<MeshRendererComponent>* MeshRenderer)
{
	ComponentInfo const* Infos[2] = { Transform, MeshRenderer };
	Filter = EntityCollection->MakeFilter(Infos);
	if (Filter) {
		TransformHandle = Filter->GetMatchComponentHandle(Transform);
		MeshRendererHandle = Filter->GetMatchComponentHandle(MeshRenderer);
		//return true;
	} else {
		//return false;
	}

	// Vulkan instance
	if (!application.Create(CTX, windowSystem->GetMainWindow())) {
		LOG(LogTemp, Error, "Failed to create the Vulkan application");
	}

	// Collect physical devices and select default one
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(application.instance, &deviceCount, nullptr);
		VkPhysicalDevice* devices = CTX.temp.Request<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(application.instance, &deviceCount, devices);

		for (int32_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
			const Rendering::VulkanPhysicalDevice physicalDevice = Rendering::VulkanPhysicalDevice::Get(CTX, devices[deviceIndex], application.surface);
			if (IsUsablePhysicalDevice(physicalDevice)) {
				availablePhysicalDevices.push_back(physicalDevice);
			}
		}

		if (availablePhysicalDevices.size() == 0) {
			LOG(LogTemp, Error, "Failed to find any vulkan physical devices");
			return false;
		}

		if (!SelectPhysicalDevice(CTX, 0)) {
			LOG(LogTemp, Error, "Failed to select default physical device");
			return false;
		}
	}

	//Create the swapchain

	//Disabled for now, something seems to wrong with the underlying system calls
	return true;
	{
		shouldRecreateSwapchain = false;
		if (!swapchain.Create(CTX, VkExtent2D{1024, 768}, application.surface, *GetPhysicalDevice(selectedPhysicalDeviceIndex), logicalDevice.device)) {
			LOG(LogTemp, Error, "Failed to create the swapchain");
			return false;
		}
	}

	return true;
}

bool RenderingSystem::Shutdown(CTX_ARG) {
	swapchain.Destroy(logicalDevice.device);
	logicalDevice.Destroy();
	application.Destroy();
	return true;
}

bool RenderingSystem::SelectPhysicalDevice(CTX_ARG, uint32_t index) {
	if (index != selectedPhysicalDeviceIndex) {
		if (const Rendering::VulkanPhysicalDevice* physicalDevice = GetPhysicalDevice(index)) {

			logicalDevice.Destroy();
			if (!logicalDevice.Create(CTX, *physicalDevice, enabledFeatures)) {
				LOGF(LogTemp, Error, "Failed to create logical device for physical device %i", index);
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

void RenderingSystem::RenderFrame(float InterpolationAlpha) const {
	for (EntityFilter<FILTER_SIZE>::FilterMatch const& Match : *Filter) {
		RenderComponent(Match.Get(MeshRendererHandle));
	}
}

void RenderingSystem::RenderComponent(MeshRendererComponent const* MeshRenderer) {
	if (!MeshRenderer->IsValid()) return;

	//glBindVertexArray(MeshRenderer->VertexArrayID);
	//glDrawArrays(GL_TRIANGLES, 0, MeshRenderer->VertexCount);
	//GLenum ErrorCode = glGetError();
	//if (ErrorCode != GL_NO_ERROR) {
	//	std::cerr << "OpenGL Error: " << ErrorCode << std::endl;
	//}
}

bool RenderingSystem::IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice) {
	return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(requiredExtensions) && physicalDevice.HasSwapchainSupport();
}
