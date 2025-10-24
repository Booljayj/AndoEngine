#include "Rendering/Vulkan/Device.h"
#include "Engine/Format.h"
#include "Engine/Logging.h"
#include "Engine/TemporaryContainers.h"

namespace Rendering {
	Device::Device(Framework const& framework, PhysicalDeviceDescription const& physical, VkPhysicalDeviceFeatures features, ExtensionsView extensions, QueueRequests const& requests)
		: physical(physical), device(nullptr)
	{
		ScopedThreadBufferMark mark;

		if (requests.size() == 0) throw std::runtime_error{ "Device was created with no queue requests. At least one must be provided." };

		//Describe how to create the queues for each family based on the requests
		t_vector<t_vector<float>> queuePriorities;
		t_vector<VkDeviceQueueCreateInfo> queueCIs;
		queuePriorities.resize(requests.size());
		queueCIs.resize(requests.size());

		for (uint32_t index = 0; index < requests.size(); ++index) {
			auto const& request = requests[index];

			queuePriorities[index].resize(request.count, 1.0f);

			queueCIs[index] = VkDeviceQueueCreateInfo{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = request.id,
				.queueCount = static_cast<uint32_t>(queuePriorities[index].size()),
				.pQueuePriorities = queuePriorities[index].data(),
			};
		}

		VkDeviceCreateInfo const deviceCI = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			//Queues
			.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size()),
			.pQueueCreateInfos = queueCIs.data(),
			//Extensions
			.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
			.ppEnabledExtensionNames = extensions.data(),
			//Features
			.pEnabledFeatures = &features,
		};

		if (vkCreateDevice(physical, &deviceCI, nullptr, &device.get()) != VK_SUCCESS || !device) {
			throw std::runtime_error{ "Failed to create command pool for logical device" };
		}

		queues = QueueResults{ device, requests };

		//Create the allocator for device memory
		VmaAllocatorCreateInfo const allocatorInfo = {
			.flags = 0,
			.physicalDevice = physical,
			.device = device,
			.instance = framework,
			.vulkanApiVersion = framework.GetMinVersion(),
		};

		if (vmaCreateAllocator(&allocatorInfo, &allocator) != VK_SUCCESS || !allocator) {
			throw std::runtime_error{ "Failed to create memory allocator for logical device" };
		}

#if VULKAN_DEBUG
		functionSetDebugName = framework.GetFunction<PFN_vkSetDebugUtilsObjectNameEXT>("vkSetDebugUtilsObjectNameEXT");
#endif
	}

	Device::~Device() {
		if (device) {
			vmaDestroyAllocator(allocator);
			vkDestroyDevice(device, nullptr);
		}
	}
}
