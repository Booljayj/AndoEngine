#pragma once
#include "Rendering/Vulkan/Framework.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/Queues.h"

namespace Rendering {
	/**
	 * Contains the components of a Vulkan logical device, which is used to communicate with a physical device.
	 * A new logical device is created for each physical device that will be used.
	 */
	struct Device {
		using ExtensionsView = TArrayView<char const* const>;
		
		/** Queues created on this device */
		QueueResults queues;

		Device(Framework const& framework, PhysicalDeviceDescription const& physical, VkPhysicalDeviceFeatures features, ExtensionsView extensions, QueueRequests const& requests);
		Device(Device const&) = delete;
		Device(Device&&) noexcept;
		~Device();

		inline operator VkDevice() const { return device; }
		inline operator VmaAllocator() const { return allocator; }

		/** Get the physical device backing this logical device */
		PhysicalDeviceDescription const& GetPhysical() const { return physical; }

#ifdef VULKAN_DEBUG
#define SET_DEBUG_NAME_IMPL(Class, type) inline VkResult SetDebugName(Class object, char const* name) const { return SetDebugName(object, type, name); }
		SET_DEBUG_NAME_IMPL(VkQueue, VK_OBJECT_TYPE_QUEUE);
		SET_DEBUG_NAME_IMPL(VkImage, VK_OBJECT_TYPE_IMAGE);
		SET_DEBUG_NAME_IMPL(VkImageView, VK_OBJECT_TYPE_IMAGE_VIEW);
#undef SET_DEBUG_NAME_IMPL
#endif

	private:
		PhysicalDeviceDescription const& physical;
		VkDevice device = nullptr;
		VmaAllocator allocator = nullptr;
#ifdef VULKAN_DEBUG
		PFN_vkSetDebugUtilsObjectNameEXT functionSetDebugName = nullptr;
#endif

#ifdef VULKAN_DEBUG
		VkResult SetDebugName(void* object, VkObjectType type, char const* name) const;
#endif
	};

	template<auto Destroyer>
	struct TUniqueHandle;

	/** Keeps track of a handle from a device and destroys it when going out of scope, unless the handle is released. */
	template<typename T, typename ReturnType, ReturnType(*Destroyer)(VkDevice, T, const VkAllocationCallbacks*)>
	struct TUniqueHandle<Destroyer> {
		TUniqueHandle(VkDevice inDevice) : device(inDevice) { if (!device) throw std::runtime_error{ "TUniqueHandle must be constructed with a valid device" }; }
		~TUniqueHandle() { if (handle) Destroyer(device, handle, nullptr); }

		inline operator bool() const { return !!handle; }
		inline operator T() const { return handle; }
		inline T const* operator*() const { return &handle; }
		inline T* operator*() { return &handle; }

		T Release() noexcept {
			T const temp = handle;
			handle = nullptr;
			return temp;
		}

	private:
		VkDevice device;
		T handle = nullptr;
	};

	template<size_t Size, auto Destroyer>
	struct TUniquePoolHandles;

	/** Keeps track of a collection of handles from a device and destroys them when going out of scope, unless the handles are released. */
	template<typename T, typename PoolType, typename ReturnType, size_t Size, ReturnType(*Deallocator)(VkDevice, PoolType, uint32_t, T const*)>
	struct TUniquePoolHandles<Size, Deallocator> {
		static_assert(Size > 0, "Pool Handles size cannot be less than 1");

		TUniquePoolHandles(VkDevice inDevice, PoolType inPool) : device(inDevice), pool(inPool), handles(nullptr) {
			if (!device || !pool) throw std::runtime_error{ "TUniquePoolHandle must be constructed with a valid device and pool" };
		}
		~TUniquePoolHandles() {
			if (handles[0]) Deallocator(device, pool, static_cast<uint32_t>(handles.size()), handles.data());
		}
	
		inline operator bool() const { return std::find(handles.begin(), handles.end(), nullptr) != handles.end(); }
		inline T operator[](size_t index) const { return handles[index]; }
		inline T const* operator*() const { return handles.data(); }
		inline T* operator*() { return handles.data(); }

		std::array<T, Size> Release() noexcept {
			std::array<T, Size> const temp = handles;
			handles.fill(nullptr);
			return temp;
		}

	private:
		VkDevice device;
		PoolType pool;
		std::array<T, Size> handles;
	};
}
