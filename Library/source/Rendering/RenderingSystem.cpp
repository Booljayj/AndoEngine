#include <cassert>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.hpp>
#include "Rendering/RenderingSystem.h"
#include "Engine/BasicComponents.h"
#include "Engine/LogCommands.h"
#include "Engine/Utility.h"
#include "EntityFramework/EntityCollectionSystem.h"
#include "Rendering/MeshRendererComponent.h"

std::ostream& operator<<(std::ostream& stream, VulkanVersion const& version) {
	stream << version.major << "." << version.minor << "." << version.patch;
	return stream;
}

VulkanPhysicalDeviceInfo VulkanPhysicalDeviceInfo::Extract(const vk::PhysicalDevice& device) {
	VulkanPhysicalDeviceInfo Result;
	Result.device = device;
	Result.deviceProperties = device.getProperties();
	Result.deviceFeatures = device.getFeatures();
	Result.deviceMemoryProperties = device.getMemoryProperties();

	return Result;
}

void VulkanPhysicalDeviceInfo::Write(std::ostream& stream) const {
	stream << "Device Name:    " << deviceProperties.deviceName << std::endl;
	stream << "Device Type:    " << vk::to_string(deviceProperties.deviceType) << std::endl;
	stream << "API Version:    " << VulkanVersion(deviceProperties.apiVersion) << std::endl;
	stream << "Driver Version: " << VulkanVersion(deviceProperties.driverVersion) << std::endl;

	stream << "Memory Heaps:  " << deviceMemoryProperties.memoryHeapCount << std::endl;
	for (size_t memoryHeapIndex = 0; memoryHeapIndex < deviceMemoryProperties.memoryHeapCount; ++memoryHeapIndex) {
		const auto& heap = deviceMemoryProperties.memoryHeaps[memoryHeapIndex];
		stream << "\tHeap " << memoryHeapIndex << ", flags: " << vk::to_string(heap.flags) << ", size: ";
		Utility::WriteByteSizeValue(std::cout, heap.size);
		stream << std::endl;
	}
	stream << "Memory Types:  " << deviceMemoryProperties.memoryTypeCount << std::endl;
	for (size_t memoryTypeIndex = 0; memoryTypeIndex < deviceMemoryProperties.memoryTypeCount; ++memoryTypeIndex) {
		const auto& type = deviceMemoryProperties.memoryTypes[memoryTypeIndex];
		stream << "\tType " << memoryTypeIndex << ", flags: " << vk::to_string(type.propertyFlags) << ", heap: " << type.heapIndex << std::endl;
	}

	stream << "Queues:" << std::endl;
	std::vector<vk::QueueFamilyProperties> const queueFamiliesProperties = device.getQueueFamilyProperties();
	for (size_t queueFamilyIndex = 0; queueFamilyIndex < queueFamiliesProperties.size(); ++queueFamilyIndex) {
		const auto& queueFamilyProperties = queueFamiliesProperties[queueFamilyIndex];
		stream << "\tFamily " << queueFamilyIndex << ", flags: " << vk::to_string(queueFamilyProperties.queueFlags) << ", count: " << queueFamilyProperties.queueCount << std::endl;
	}
}

bool RenderingSystem::Startup(
	CTX_ARG,
	EntityCollectionSystem* EntityCollection,
	TComponentInfo<TransformComponent>* Transform,
	TComponentInfo<MeshRendererComponent>* MeshRenderer)
{
	ComponentInfo const* Infos[] = { Transform, MeshRenderer };
	Filter = EntityCollection->MakeFilter(Infos);
	if (Filter) {
		TransformHandle = Filter->GetMatchComponentHandle(Transform);
		MeshRendererHandle = Filter->GetMatchComponentHandle(MeshRenderer);
		//return true;
	} else {
		//return false;
	}

	{
		// Vulkan instance
		vk::ApplicationInfo appInfo;
		appInfo.pApplicationName = "DefaultProject";
		appInfo.pEngineName = "AndoEngine";
		appInfo.apiVersion = VK_API_VERSION_1_0;

		vk::InstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.pApplicationInfo = &appInfo;
		instance = vk::createInstance(instanceCreateInfo);
	}

	// Physical device
	availablePhysicalDevices = instance.enumeratePhysicalDevices();

	if (availablePhysicalDevices.size() == 0) {
		LOG(LogTemp, Error, "Failed to find any vulkan physical devices");
		return false;
	}

	selectedPhysicalDeviceInfo = VulkanPhysicalDeviceInfo::Extract(availablePhysicalDevices[0]);
	selectedPhysicalDeviceInfo.Write(std::cout);

	return true;
}

bool RenderingSystem::Shutdown(CTX_ARG) {
	if (!!instance) instance.destroy();
	return true;
}

void RenderingSystem::RenderFrame(float InterpolationAlpha) const {
	for (EntityFilter<FILTER_SIZE>::FilterMatch const& Match : *Filter) {
		RenderComponent(Match.Get(MeshRendererHandle));
	}
}

void RenderingSystem::RenderComponent(MeshRendererComponent const* MeshRenderer) {
	if (!MeshRenderer->IsValid()) return;

	glBindVertexArray(MeshRenderer->VertexArrayID);
	glDrawArrays(GL_TRIANGLES, 0, MeshRenderer->VertexCount);

	GLenum ErrorCode = glGetError();
	if (ErrorCode != GL_NO_ERROR) {
		std::cerr << "OpenGL Error: " << ErrorCode << std::endl;
	}
}
