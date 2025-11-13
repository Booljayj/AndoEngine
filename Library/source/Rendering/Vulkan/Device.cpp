#include "Rendering/Vulkan/Device.h"
#include "Engine/Format.h"
#include "Engine/Logging.h"
#include "Engine/TemporaryContainers.h"

namespace Rendering {
	Device::Device(Framework const& framework, PhysicalDeviceDescription const& physical, PhysicalDeviceFeatures const& enabled_features, NameSpan enabled_extension_names, QueueRequests const& requests)
		: physical(physical), device(nullptr)
	{
		ScopedThreadBufferMark mark;

		for (char const* extension_name : enabled_extension_names) {
			if (!physical.SupportsExtension(extension_name)) throw FormatType<std::runtime_error>("Enabled extension {} is not supported by physical device {}", extension_name, physical.properties.deviceName);
		}

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
			.pNext = &enabled_features.version10,
			//Queues
			.queueCreateInfoCount = static_cast<uint32_t>(queueCIs.size()),
			.pQueueCreateInfos = queueCIs.data(),
			//Extensions
			.enabledExtensionCount = static_cast<uint32_t>(enabled_extension_names.size()),
			.ppEnabledExtensionNames = enabled_extension_names.data(),
			//Features
			.pEnabledFeatures = nullptr, //This is handled with VkPhysicalDeviceFeatures2 in the pNext chain
		};

		if (vkCreateDevice(physical, &deviceCI, nullptr, &device.get()) != VK_SUCCESS || !device) {
			throw std::runtime_error{ "Failed to create logical device" };
		}

		queues = QueueResults{ device, requests };

		//Create the allocator for device memory
		VmaAllocatorCreateInfo const allocatorInfo = {
			.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
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
