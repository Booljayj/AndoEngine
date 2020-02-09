#include "Engine/LinearContainers.h"
#include "Engine/ScopedTempBlock.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"

namespace Rendering {
	bool VulkanLogicalDevice::Create(CTX_ARG, VulkanPhysicalDevice const& physicalDevice, VkPhysicalDeviceFeatures const& inEnabledFeatures) {
		TEMP_SCOPE;
		enabledFeatures = inEnabledFeatures;

		l_unordered_set<uint32_t> uniqueQueueFamilies{CTX.Temp};
		uniqueQueueFamilies.insert(physicalDevice.queues.graphics.value().index);
		uniqueQueueFamilies.insert(physicalDevice.queues.present.value().index);

		l_vector<uint32_t> queueCreateInfoIndices{uniqueQueueFamilies.begin(), uniqueQueueFamilies.end(), CTX.Temp};
		uint32_t const queueCICount = queueCreateInfoIndices.size();

		float queuePriority = 1.0f;

		VkDeviceQueueCreateInfo* const queueCIs = CTX.Temp.Request<VkDeviceQueueCreateInfo>(queueCICount);
		for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueCICount; ++queueFamilyIndex) {
			VkDeviceQueueCreateInfo& queueCI = queueCIs[queueFamilyIndex];
			queueCI = {};
			queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCI.queueFamilyIndex = queueCreateInfoIndices[queueFamilyIndex];
			queueCI.queueCount = 1; //although the family may support more than one queue, it's usually only necessary to create one.
			queueCI.pQueuePriorities = &queuePriority;
		}

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = queueCICount;
		deviceCreateInfo.pQueueCreateInfos = queueCIs;
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

		vkCreateDevice(physicalDevice.device, &deviceCreateInfo, nullptr, &device);

		if (device) {
			vkGetDeviceQueue(device, physicalDevice.queues.graphics.value().index, 0, &queues.graphics);
			vkGetDeviceQueue(device, physicalDevice.queues.present.value().index, 0, &queues.present);
			return true;
		} else {
			return false;
		}
	}

	void VulkanLogicalDevice::Destroy() {
		if (!!device) vkDestroyDevice(device, nullptr);
	}
}
