#include "Engine/LinearContainers.h"
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

		other.device = nullptr;
		other.queues.graphics = nullptr;
		other.queues.present = nullptr;

		return *this;
	}

	VulkanLogicalDevice VulkanLogicalDevice::Create(CTX_ARG, VulkanPhysicalDevice const& physicalDevice, VkPhysicalDeviceFeatures const& enabledFeatures, TArrayView<char const*> const& enabledExtensionNames) {
		TEMP_ALLOCATOR_MARK();

		VulkanLogicalDevice result;

		l_unordered_set<uint32_t> uniqueQueueFamilies{CTX.temp};
		uniqueQueueFamilies.insert(physicalDevice.queues.graphics.value().index);
		uniqueQueueFamilies.insert(physicalDevice.queues.present.value().index);

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

		vkCreateDevice(physicalDevice.device, &deviceCI, nullptr, &result.device);

		if (result.device) {
			vkGetDeviceQueue(result.device, physicalDevice.queues.graphics.value().index, 0, &result.queues.graphics);
			vkGetDeviceQueue(result.device, physicalDevice.queues.present.value().index, 0, &result.queues.present);
		}

		return result;
	}

	void VulkanLogicalDevice::Destroy() {
		if (!!device) {
			vkDestroyDevice(device, nullptr);
			device = nullptr;
			queues.present = nullptr;
			queues.graphics = nullptr;
		}
	}
}
