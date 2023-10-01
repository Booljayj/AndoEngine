#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Surface.h"
#include "Rendering/Vulkan/PhysicalDevice.h"

namespace Rendering {
	Swapchain::Swapchain(VkDevice inDevice, Swapchain* previous, PhysicalDevicePresentation const& presentation, PhysicalDeviceCapabilities const& capabilities, Surface const& surface)
		: device(inDevice)
	{
		auto const ChooseSwapSurfaceFormat = [](const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats) -> VkSurfaceFormatKHR {
			if (availableSurfaceFormats.size() == 0) {
				throw std::runtime_error{ "Physical device does not provide any avialable surface formats" };
			}

			for (const auto& availableSurfaceFormat : availableSurfaceFormats) {
				if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					return availableSurfaceFormat;
				}
			}
			return availableSurfaceFormats[0];
		};

		auto const ChooseSwapPresentMode = [](const std::vector<VkPresentModeKHR>& availablePresentModes) -> VkPresentModeKHR {
			for (const auto& availablePresentMode : availablePresentModes) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
					return availablePresentMode;
				}
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		};

		surfaceFormat = ChooseSwapSurfaceFormat(presentation.surfaceFormats);
		presentMode = ChooseSwapPresentMode(presentation.presentModes);
		extent = capabilities.GetSwapExtent(surface, extent);
		preTransform = capabilities.GetPreTransform(surface);

		VkSwapchainCreateInfoKHR swapchainCI = {};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.surface = surface;
		//Image settings
		swapchainCI.minImageCount = capabilities.GetImageCountMinimum();
		swapchainCI.imageFormat = surfaceFormat.format;
		swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
		swapchainCI.imageExtent = VkExtent2D{ extent.x, extent.y };
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		//Queue settings
		uint32_t const queueFamilyIndices[2] = { surface.queues->graphics.index, surface.queues->present.index };
		if (queueFamilyIndices[0] == queueFamilyIndices[1]) {
			swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCI.queueFamilyIndexCount = 0;
			swapchainCI.pQueueFamilyIndices = nullptr;
		} else {
			swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapchainCI.queueFamilyIndexCount = 2;
			swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
		}
		//Transform settings
		swapchainCI.preTransform = preTransform;
		//Compositing settings
		swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		//Presentation settings
		swapchainCI.presentMode = presentMode;
		swapchainCI.clipped = true;
		//The old swapchain that is being replaced, which will be destroyed after the new one is created.
		swapchainCI.oldSwapchain = previous ? *previous : nullptr;

		VkSwapchainKHR tempSwapchain = nullptr;
		if (vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &tempSwapchain) != VK_SUCCESS || !tempSwapchain) {
			throw std::runtime_error{ "Unable to create swapchain" };
		}

		uint32_t numImages = 0;
		vkGetSwapchainImagesKHR(device, tempSwapchain, &numImages, nullptr);
		if (numImages == 0) {
			vkDestroySwapchainKHR(device, tempSwapchain, nullptr);
			throw std::runtime_error{ "Swapchain has no images to retrieve" };
		}

		swapchain = tempSwapchain;
		images.resize(numImages);
		vkGetSwapchainImagesKHR(device, swapchain, &numImages, images.data());
	}

	Swapchain::Swapchain(Swapchain&& other) noexcept
		: device(other.device), swapchain(other.swapchain), images(std::move(other.images))
		, surfaceFormat(other.surfaceFormat), presentMode(other.presentMode), extent(other.extent), preTransform(other.preTransform)
	{
		other.device = nullptr;
	}

	Swapchain::~Swapchain() {
		if (device) {
			vkDeviceWaitIdle(device);
			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}
	}
}
