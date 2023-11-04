#include "Rendering/Vulkan/Device.h"
#include "Engine/Logging.h"
#include "Engine/Temporary.h"

namespace Rendering {
	Device::Device(Framework const& framework, PhysicalDeviceDescription const& physical, VkPhysicalDeviceFeatures features, ExtensionsView extensions, QueueRequests const& requests)
		: physical(physical)
	{
		ScopedThreadBufferMark mark;

		if (requests.Size() == 0) throw std::runtime_error{ "Device was created with no queue requests. At least one must be provided." };

		float queuePriority = 1.0f;

		//Describe how to create the queues for each family based on the requests
		t_vector<VkDeviceQueueCreateInfo> queueCIs;
		queueCIs.resize(requests.Size());
		for (uint32_t req = 0; req < requests.Size(); ++req) {
			auto const& request = requests[req];

			VkDeviceQueueCreateInfo& queueCI = queueCIs[req];
			queueCI = {};
			queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCI.queueFamilyIndex = request.family;
			queueCI.queueCount = request.count;
			queueCI.pQueuePriorities = &queuePriority;
		}

		VkDeviceCreateInfo deviceCI = {};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//Queues
		deviceCI.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size());
		deviceCI.pQueueCreateInfos = queueCIs.data();
		//Features
		deviceCI.pEnabledFeatures = &features;
		//Extensions
		deviceCI.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		deviceCI.ppEnabledExtensionNames = extensions.data();

		if (vkCreateDevice(physical, &deviceCI, nullptr, &device) != VK_SUCCESS || !device) {
			throw std::runtime_error{ "Failed to create command pool for logical device" };
		}

		queues = QueueResults{ device, requests };

		//Create the allocator for device memory
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physical;
		allocatorInfo.device = device;
		allocatorInfo.instance = framework;
		allocatorInfo.vulkanApiVersion = framework.GetMinVersion();
		allocatorInfo.flags = 0;

		if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS || !allocator) {
			throw std::runtime_error{ "Failed to create memory allocator for logical device" };
		}

#if VULKAN_DEBUG
		functionSetDebugName = framework.GetFunction<PFN_vkSetDebugUtilsObjectNameEXT>("vkSetDebugUtilsObjectNameEXT");
#endif
	}

	Device::Device(Device&& other) noexcept
		: queues(std::move(other.queues)), physical(other.physical), device(other.device), allocator(other.allocator)
#if VULKAN_DEBUG
		, functionSetDebugName(other.functionSetDebugName)
#endif
	{
		other.device = nullptr;
	}

	Device::~Device() {
		if (device) {
			vmaDestroyAllocator(allocator);
			vkDestroyDevice(device, nullptr);
		}
	}
}
