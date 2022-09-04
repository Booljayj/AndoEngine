#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Engine/Logging.h"
#include "Engine/Temporary.h"

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

	VulkanLogicalDevice VulkanLogicalDevice::Create(VulkanFramework framework, VulkanPhysicalDevice const& physical, VkPhysicalDeviceFeatures const& enabledFeatures, TArrayView<char const*> const& enabledExtensionNames) {
		SCOPED_TEMPORARIES();

		VulkanLogicalDevice result;

		t_unordered_set<uint32_t> uniqueQueueFamilies{ 2 };
		uniqueQueueFamilies.insert(physical.queues.graphics.value().index);
		uniqueQueueFamilies.insert(physical.queues.present.value().index);

		t_vector<uint32_t> const queueCreateInfoIndices{ uniqueQueueFamilies.begin(), uniqueQueueFamilies.end() };
		uint32_t const queueCICount = queueCreateInfoIndices.size();

		float queuePriority = 1.0f;

		t_vector<VkDeviceQueueCreateInfo> queueCIs{ queueCICount };
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
		deviceCI.queueCreateInfoCount = queueCIs.size();
		deviceCI.pQueueCreateInfos = queueCIs.data();
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

#if VULKAN_DEBUG
		result.functionSetDebugName = framework.GetFunction<PFN_vkSetDebugUtilsObjectNameEXT>("vkSetDebugUtilsObjectNameEXT");
		result.SetDebugName(result.queues.graphics, "graphics queue");
		result.SetDebugName(result.queues.present, "present queue");
#endif

		return result;
	}

	void VulkanLogicalDevice::Destroy() {
		vmaDestroyAllocator(allocator);
		if (device) vkDestroyDevice(device, nullptr);
		allocator = nullptr;
		queues.present = nullptr;
		queues.graphics = nullptr;
		device = nullptr;
#ifdef VULKAN_DEBUG
		functionSetDebugName = nullptr;
#endif
	}

#ifdef VULKAN_DEBUG
	VkResult VulkanLogicalDevice::SetDebugName(void* object, VkObjectType type, char const* name) const {
		if (functionSetDebugName) {
			VkDebugUtilsObjectNameInfoEXT info{};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			info.objectType = type;
			info.objectHandle  = reinterpret_cast<uint64_t>(object);
			info.pObjectName = name;
			return functionSetDebugName(device, &info);
		}
		else return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
#endif

	VmaAllocatorCreateFlags VulkanLogicalDevice::GetAllocatorFlags() {
		return 0;
	}
}
