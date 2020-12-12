#include "Engine/LinearContainers.h"
#include "Engine/LogCommands.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"

namespace Rendering {
	VulkanLogicalDevice::VulkanLogicalDevice(VulkanLogicalDevice&& other) {
		*this = std::move(other);
	}

	VulkanLogicalDevice& VulkanLogicalDevice::operator=(VulkanLogicalDevice&& other) {
		Destroy();

		device = other.device;
		queues.graphics = other.queues.graphics;
		queues.present = other.queues.present;
		allocator = other.allocator;

		other.device = nullptr;
		other.queues.graphics = nullptr;
		other.queues.present = nullptr;
		other.allocator = nullptr;

		return *this;
	}

	VulkanLogicalDevice VulkanLogicalDevice::Create(CTX_ARG, VulkanFramework framework, VulkanPhysicalDevice const& physical, VkPhysicalDeviceFeatures const& enabledFeatures, TArrayView<char const*> const& enabledExtensionNames) {
		TEMP_ALLOCATOR_MARK();

		VulkanLogicalDevice result;

		l_unordered_set<uint32_t> uniqueQueueFamilies{CTX.temp};
		uniqueQueueFamilies.insert(physical.queues.graphics.value().index);
		uniqueQueueFamilies.insert(physical.queues.present.value().index);

		l_vector<uint32_t> queueCreateInfoIndices{uniqueQueueFamilies.begin(), uniqueQueueFamilies.end(), CTX.temp};
		uint32_t const queueCICount = queueCreateInfoIndices.size();

		float queuePriority = 1.0f;

		VkDeviceQueueCreateInfo* const queueCIs = CTX.temp.Request<VkDeviceQueueCreateInfo>(queueCICount);
		for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueCICount; ++queueFamilyIndex) {
			VkDeviceQueueCreateInfo& queueCI = queueCIs[queueFamilyIndex];
			queueCI = {};
			queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCI.queueFamilyIndex = queueCreateInfoIndices[queueFamilyIndex];
			queueCI.queueCount = 1; //although the family may support more than one queue, it's usually only necessary to create one.
			queueCI.pQueuePriorities = &queuePriority;
		}

		VkDeviceCreateInfo deviceCI = {};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//Queues
		deviceCI.queueCreateInfoCount = queueCICount;
		deviceCI.pQueueCreateInfos = queueCIs;
		//Features
		deviceCI.pEnabledFeatures = &enabledFeatures;
		//Extensions
		deviceCI.enabledExtensionCount = enabledExtensionNames.size();
		deviceCI.ppEnabledExtensionNames = enabledExtensionNames.begin();

		assert(!result.device);
		if (vkCreateDevice(physical.device, &deviceCI, nullptr, &result.device) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create command pool for logical device");
			return result;
		}

		vkGetDeviceQueue(result.device, physical.queues.graphics.value().index, 0, &result.queues.graphics);
		vkGetDeviceQueue(result.device, physical.queues.present.value().index, 0, &result.queues.present);

		//Create the allocator for device memory
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = physical.device;
		allocatorInfo.device = result.device;
		allocatorInfo.instance = framework.instance;
		allocatorInfo.vulkanApiVersion = framework.version;
		allocatorInfo.flags = GetAllocatorFlags();

		if (vmaCreateAllocator(&allocatorInfo, &result.allocator) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create memory allocator for logical device");
			return result;
		}

		return result;
	}

	void VulkanLogicalDevice::Destroy() {
		vmaDestroyAllocator(allocator);
		if (device) vkDestroyDevice(device, nullptr);
		allocator = nullptr;
		queues.present = nullptr;
		queues.graphics = nullptr;
		device = nullptr;
	}

	VmaAllocatorCreateFlags VulkanLogicalDevice::GetAllocatorFlags() {
		return 0;
	}
}
