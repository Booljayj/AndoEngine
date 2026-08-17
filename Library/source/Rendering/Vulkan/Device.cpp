#include "Rendering/Vulkan/Device.h"
#include "Engine/Format.h"
#include "Engine/Logging.h"
#include "Engine/TemporaryContainers.h"

namespace Rendering {
	std::optional<SharedQueues> Device::GetSharedQueues(SharedQueues::References const& references) const
	{
		SharedQueues result;

		for (QueueReference transfer : references.transfers) {
			if (VkQueue queue = FindQueue(transfer)) result.transfers.emplace_back(queue, transfer);
			else return std::nullopt;
		}
		for (QueueReference compute : references.computes) {
			if (VkQueue queue = FindQueue(compute)) result.computes.emplace_back(queue, compute);
			else return std::nullopt;
		}

		return result;
	}

	std::optional<SurfaceQueues> Device::GetSurfaceQueues(SurfaceQueues::References const& references) const
	{
		VkQueue present_queue = FindQueue(references.present);
		if (!present_queue) throw std::runtime_error{ "Failed to resolve present queue on device" };

		VkQueue graphics_queue = FindQueue(references.graphics);
		if (!graphics_queue) throw std::runtime_error{ "Failed to resolve graphics queue on device" };

		return SurfaceQueues{
			PresentQueue{ present_queue, references.present },
			GraphicsQueue{ graphics_queue, references.graphics }
		};
	}

	Device::Device(Framework const& framework, PhysicalDeviceDescription const& physical, Features enabled_features, NameSpan enabled_extension_names, SharedQueues::References const& shared_references)
		: Device(framework, physical, enabled_features, enabled_extension_names, GenerateQueueRequests(shared_references))
	{
		const auto possible_shared = GetSharedQueues(shared_references);
		if (!possible_shared) throw std::runtime_error{ "Failed to retrieve shared queues on device" };

		queues.shared = *possible_shared;
	}

	Device::Device(Framework const& framework, PhysicalDeviceDescription const& physical, Features enabled_features, NameSpan enabled_extension_names, SharedQueues::References const& shared_references, SurfaceQueues::References const& surface_references)
		: Device(framework, physical, enabled_features, enabled_extension_names, GenerateQueueRequests(shared_references, surface_references))
	{
		const auto possible_shared = GetSharedQueues(shared_references);
		if (!possible_shared) throw std::runtime_error{ "Failed to retrieve shared queues on device" };

		queues.shared = *possible_shared;
		queues.surface = GetSurfaceQueues(surface_references);
	}

	Device::Device(Framework const& framework, PhysicalDeviceDescription const& physical, Features enabled_features, NameSpan enabled_extension_names, QueueRequests const& requests)
		: device(nullptr)
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

	VkQueue Device::FindQueue(QueueReference reference) const
	{
		VkQueue queue = nullptr;
		vkGetDeviceQueue(device, reference.id, reference.index, &queue);
		return queue;
	}

	QueueRequests Device::GenerateQueueRequests(SharedQueues::References const& shared_references)
	{
		QueueRequests requests;
		for (auto const& transfer : shared_references.transfers) requests += transfer;
		for (auto const& compute : shared_references.computes) requests += compute;
		return requests;
	}

	QueueRequests Device::GenerateQueueRequests(SharedQueues::References const& shared_references, SurfaceQueues::References const& surface_references)
	{
		QueueRequests requests = GenerateQueueRequests(shared_references);
		requests += surface_references.present;
		requests += surface_references.graphics;
		return requests;
	}

#if VULKAN_DEBUG
	VkResult Device::SetDebugName(void* object, VkObjectType type, char const* name) const
	{
		if (functionSetDebugName)
		{
			VkDebugUtilsObjectNameInfoEXT info = {};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			info.pNext = nullptr;
			info.objectHandle = reinterpret_cast<uint64_t>(object);
			info.objectType = type;
			info.pObjectName = name;
			return functionSetDebugName(device, &info);
		}
		return VkResult::VK_ERROR_EXTENSION_NOT_PRESENT;
	}
#endif

	Device::~Device() {
		if (device) {
			vmaDestroyAllocator(allocator);
			vkDestroyDevice(device, nullptr);
		}
	}
}
