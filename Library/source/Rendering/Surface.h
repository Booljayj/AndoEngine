#pragma once
#include "Engine/StandardTypes.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Vulkan/FrameOrganizer.h"
#include "Rendering/Vulkan/RenderPasses.h"
#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "ThirdParty/EnTT.h"

namespace Rendering {
	struct RenderingSystem;

	/** A surface tied to a given window, which is used by a RenderSystem for rendering */
	struct Surface {
	public:
		/** The maximum number of consecutive times we can fail to render a frame */
		static constexpr uint8_t maxRetryCount = 5;

		Surface(VkInstance instance, HAL::Window& inWindow);
		Surface(Surface const&) = delete;
		Surface(Surface&&) = delete;
		~Surface();

		operator VkSurfaceKHR() const { return surface; }
		inline bool operator==(HAL::Window::IdType otherID) const { return GetID() == otherID; }

		/** Get the id of the window associated with this surface */
		inline HAL::Window::IdType GetID() const { return window.id; }

		/** Return whether this is a valid surface that is prepared for rendering */
		inline bool CanRender() const { return surface && queues && swapchain; }
		/** Whether the swapchain needs to be recreated before it is used again */
		inline bool IsSwapchainDirty() const { return shouldRecreateSwapchain; }

		/** Create resources used for rendering */
		void InitializeRendering(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& layouts);
		/** Remove resources used for rendering */
		void DeinitializeRendering();

		/** Create or recreate the swapchain and related resources for this surface */
		bool RecreateSwapchain(Device const& device, PhysicalDeviceDescription const& physical, RenderPasses const& passes, UniformLayouts const& layouts);
		
		/** Render the renderable entities from the registry to this surface */
		bool Render(RenderPasses const& passes, entt::registry& registry);

	private:
		friend RenderingSystem;
		friend Swapchain;

		VkInstance instance = nullptr;
		HAL::Window& window;

		/** The internal surface tied to this surface */
		VkSurfaceKHR surface = nullptr;

		/** The queues that this surface will use to submit commands */
		std::optional<SurfaceQueues> queues;

		/** The swapchain that is currently being used for images */
		std::optional<Swapchain> swapchain;
		/** The framebuffers for rendering on the swapchain */
		std::optional<Framebuffers> framebuffers;
		/** The frame organizer that keeps track of resources used each frame */
		std::optional<FrameOrganizer> organizer;

		/** The full size of the window that is associated with this surface */
		glm::u32vec2 windowSize;

		uint8_t retryCount : 1;
		uint8_t shouldRecreateSwapchain : 1;
	};
}
