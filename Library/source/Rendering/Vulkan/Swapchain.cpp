#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Surface.h"
#include "Rendering/Vulkan/PhysicalDevice.h"

namespace Rendering {
	Swapchain::Swapchain(VkDevice device, Swapchain* previous, PhysicalDevicePresentation const& presentation, PhysicalDeviceCapabilities const& capabilities, Surface const& surface)
		: device(device)
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

		uint32_t const queueFamilyIndices[2] = { surface.queues->graphics.index, surface.queues->present.index };
		bool const usingSharedGraphicsAndPresentQueues = (queueFamilyIndices[0] == queueFamilyIndices[1]);

		VkSwapchainCreateInfoKHR const swapchainCI = usingSharedGraphicsAndPresentQueues ?
			VkSwapchainCreateInfoKHR{
				.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface = surface,
				//Image settings
				.minImageCount = capabilities.GetImageCountMinimum(),
				.imageFormat = surfaceFormat.format,
				.imageColorSpace = surfaceFormat.colorSpace,
				.imageExtent = VkExtent2D{ extent.x, extent.y },
				.imageArrayLayers = 1,
				.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				//Queue settings
				.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
				//Presentation settings
				.preTransform = preTransform,
				.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = presentMode,
				.clipped = true,
				//The old swapchain that is being replaced, which will be destroyed after the new one is created.
				.oldSwapchain = previous ? *previous : nullptr,
			} :
			VkSwapchainCreateInfoKHR{
				.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.surface = surface,
				//Image settings
				.minImageCount = capabilities.GetImageCountMinimum(),
				.imageFormat = surfaceFormat.format,
				.imageColorSpace = surfaceFormat.colorSpace,
				.imageExtent = VkExtent2D{ extent.x, extent.y },
				.imageArrayLayers = 1,
				.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				//Queue settings
				.imageSharingMode = VK_SHARING_MODE_CONCURRENT,
				.queueFamilyIndexCount = 2,
				.pQueueFamilyIndices = queueFamilyIndices,
				//Presentation settings
				.preTransform = preTransform,
				.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = presentMode,
				.clipped = true,
				//The old swapchain that is being replaced, which will be destroyed after the new one is created.
				.oldSwapchain = previous ? *previous : nullptr,
			};
		
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

	Swapchain::~Swapchain() {
		if (device) {
			vkDeviceWaitIdle(device);
			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}
	}
}
