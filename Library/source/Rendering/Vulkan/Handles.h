#pragma once
#include "Engine/Core.h"
#include "Engine/UniqueResource.h"
#include "Rendering/Vulkan/Vulkan.h"

/*
 * Contains methods and types to help create owning handles to Vulkan objects,
 * which will automatically clean those objects up if they aren't consumed or manually destroyed in some way.
 */

namespace Rendering {
	/** Creates handles to objects that are owned by a device */
	template<typename T, typename ParamsType, auto* CreateMethod, auto* DeleteMethod>
	struct DeviceOwned {
		struct Deleter {
			static constexpr T NullResourceValue = nullptr;

			Deleter(VkDevice device) : device(device) {}
			Deleter(Deleter const&) = default;
			void operator()(T handle) const { DeleteMethod(device, handle, nullptr); }

		private:
			VkDevice device;
		};

		template<typename... ExceptionArgTypes>
		static TUniqueResource<T, Deleter> Create(VkDevice device, ParamsType const& params, std::format_string<ExceptionArgTypes...> format, ExceptionArgTypes&&... arguments) {
			T handle = nullptr;
			if (CreateMethod(device, &params, nullptr, &handle) != VK_SUCCESS || !handle) {
				throw FormatType<std::runtime_error>(format, std::forward<ExceptionArgTypes>(arguments)...);
			}
			return TUniqueResource<T, Deleter>{ handle, Deleter{ device } };
		}
	};

	using ImageView = DeviceOwned<VkImageView, VkImageViewCreateInfo, &vkCreateImageView, &vkDestroyImageView>;
	using Semaphore = DeviceOwned<VkSemaphore, VkSemaphoreCreateInfo, &vkCreateSemaphore, &vkDestroySemaphore>;

	/** Creates handles to objects that are owned by a pool within a device */
	template<typename T, typename PoolType, typename ParamsType, auto* CreateMethod, auto* DeleteMethod>
	struct PoolOwned {
		struct Deleter {
			static constexpr T NullResourceValue = nullptr;

			Deleter(VkDevice device, PoolType pool) : device(device), pool(pool) {}
			Deleter(Deleter const&) = default;
			void operator()(std::span<T const> handles) const { DeleteMethod(device, pool, static_cast<uint32_t>(handles.size()), handles.data()); }

		private:
			VkDevice device;
			PoolType pool;
		};

		template<size_t Size, typename... ExceptionArgTypes>
		static TUniqueResources<T, Size, Deleter> Create(VkDevice device, PoolType pool, ParamsType const& params, std::format_string<ExceptionArgTypes...> format, ExceptionArgTypes&&... arguments) {
			std::array<T, Size> handles;
			handles.fill(nullptr);
			if (CreateMethod(device, &params, handles.data()) != VK_SUCCESS || !handles[0]) {
				throw FormatType<std::runtime_error>(format, std::forward<ExceptionArgTypes>(arguments)...);
			}
			return TUniqueResources<T, Size, Deleter>{ handles, Deleter{ device, pool } };
		}
	};

	using CommandBuffers = PoolOwned<VkCommandBuffer, VkCommandPool, VkCommandBufferAllocateInfo,&vkAllocateCommandBuffers, &vkFreeCommandBuffers>;
}
