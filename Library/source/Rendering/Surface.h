#pragma once
#include "Engine/StandardTypes.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "ThirdParty/EnTT.h"

namespace Rendering {
	struct RenderingSystem;

	/** A surface tied to a given window, which is used by a RenderSystem for rendering */
	struct Surface {
	public:
		/** The maximum number of consecutive times we can fail to render a frame */
		static constexpr uint8_t maxRetryCount = 5;

		/** The internal surface tied to this surface */
		VkSurfaceKHR surface = nullptr;

		/** The swapchain that is currently being used for images */
		std::optional<VulkanSwapchain> swapchain;
		/** The framebuffers for rendering on the swapchain */
		std::optional<VulkanFramebuffers> framebuffers;

		/** The frame organizer that keeps track of resources used each frame */
		std::optional<VulkanFrameOrganizer> organizer;

		Surface(RenderingSystem& inOwner, HAL::Window& inWindow);
		~Surface();

		operator VkSurfaceKHR() const { return surface; }
		inline bool operator==(HAL::Window::IdType otherID) const { return GetID() == otherID; }

		/** Return whether this is a valid surface, which can potentially be used for rendering. It may not be fully set up for rendering yet */
		inline bool IsValid() const { return surface; }
		/** Get the id of the window associated with this surface */
		inline HAL::Window::IdType GetID() const { return window.id; }

		/** Return whether this is a valid surface that is prepared for rendering */
		inline bool CanRender() const { return surface && swapchain; }
		/** Whether the swapchain needs to be recreated before it is used again */
		inline bool IsSwapchainDirty() const { return shouldRecreateSwapchain; }

		/** Get information about how a physical device can be used with this surface */
		VulkanPhysicalDevice GetPhysicalDevice(VkPhysicalDevice device);
		/** Returns whether the physical device can be used to render to this surface */
		bool IsPhysicalDeviceUsable(VulkanPhysicalDevice const& physical) const;
		/** Get the preferred surface format when rendering to this surface with the given physical device */
		VkSurfaceFormatKHR GetPreferredSurfaceFormat(VulkanPhysicalDevice const& physical);

		bool CreateSwapchain(VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes);
		bool RecreateSwapchain(VulkanPhysicalDevice const& physical, VkSurfaceFormatKHR surfaceFormat, VulkanRenderPasses const& passes);
		void Destroy(VulkanFramework const& framework, VulkanLogicalDevice const& logical);

		bool Render(VulkanLogicalDevice const& logical, VulkanRenderPasses const& passes, entt::registry& registry);

		/** Change the size of this surface */
		void Resize(glm::u32vec2 const& newSize);

	private:
		RenderingSystem& owner;
		HAL::Window& window;
		EventHandleType windowDestroyedHandle;

		/** The full size of the window that is associated with this surface */
		glm::u32vec2 windowSize;
		/** The actual drawable size of swapchain images created for this surface */
		glm::u32vec2 imageSize;

		uint8_t retryCount : 1;
		uint8_t shouldRecreateSwapchain : 1;

		void OnWindowDestroyed(HAL::Window::IdType id);
	};
}
