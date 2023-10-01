#pragma once
#include "Engine/StandardTypes.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct PhysicalDeviceCapabilities;
	struct PhysicalDevicePresentation;
	struct Surface;

	/**
	 * Contains the components of a Vulkan swapchain, which are used to communicate with the graphics device.
	 * These components are recreated when devices or rendering parameters change (such as screen size).
	 */
	struct Swapchain {
	public:
		Swapchain(VkDevice inDevice, Swapchain* previous, PhysicalDevicePresentation const& presentation, PhysicalDeviceCapabilities const& capabilities, Surface const& surface);
		Swapchain(Swapchain const&) = delete;
		Swapchain(Swapchain&&) noexcept;
		~Swapchain();

		inline operator VkSwapchainKHR() const { return swapchain; }
		
		inline VkSurfaceFormatKHR GetSurfaceFormat() const { return surfaceFormat; }
		inline VkPresentModeKHR GetPresentMode() const { return presentMode; }
		inline glm::u32vec2 GetExtent() const { return extent; }
		inline VkSurfaceTransformFlagBitsKHR GetPreTransform() const { return preTransform; }

		template<typename NumType>
		inline void GetExtent(NumType& x, NumType& y) const { x = static_cast<NumType>(extent.x); y = static_cast<NumType>(extent.y); }

		inline uint32_t GetNumImages() const { return static_cast<uint32_t>(images.size()); }
		inline std::vector<VkImage> const& GetImages() const { return images; }

		friend void swap(Rendering::Swapchain& lhs, Rendering::Swapchain& rhs) {
			std::swap(lhs.device, rhs.device);
			std::swap(lhs.swapchain, rhs.swapchain);
			std::swap(lhs.images, rhs.images);
			std::swap(lhs.surfaceFormat, rhs.surfaceFormat);
			std::swap(lhs.presentMode, rhs.presentMode);
			std::swap(lhs.extent, rhs.extent);
			std::swap(lhs.preTransform, rhs.preTransform);
		}

	private:
		VkDevice device = nullptr;
		VkSwapchainKHR swapchain = nullptr;

		/** The images in the swapchain */
		std::vector<VkImage> images;

		VkSurfaceFormatKHR surfaceFormat = {};
		VkPresentModeKHR presentMode = {};
		glm::u32vec2 extent = { 1, 1 };
		VkSurfaceTransformFlagBitsKHR preTransform = {};
	};
}
