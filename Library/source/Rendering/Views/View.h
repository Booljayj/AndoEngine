#pragma once
#include "Engine/StandardTypes.h"
#include "Geometry/ScreenRect.h"
#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Vulkan/FrameOrganizer.h"
#include "Rendering/Vulkan/Device.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Rendering/Vulkan/VulkanResourcesHelpers.h"
#include "ThirdParty/EnTT.h"

namespace Rendering {
	struct ViewPerspectiveMatrixParams {
		/** The transform that provides the location, rotation, and scale of the matrix */
		glm::mat4 transform;
		/** The vertical field-of-view */
		float fov;
		/** The aspect ratio to render with */
		float aspect;
		/** The near and far clip distances from the matrix location */
		struct {
			float near;
			float far;
		} clip;
	};

	/** A view renders things within a region, using some configuration of viewports and render passes */
	struct View {
		View(Geometry::ScreenRect const& inRect);
		virtual ~View() = default;

		/** Prepare to render commands for this view for a single frame */
		virtual void Prepare(entt::registry const& registry, FrameResources& frame) const = 0;
		/** Record rendering commands for this view for a single frame */
		virtual void Record(const entt::registry& registry, const FrameResources& frame, size_t index) const = 0;

		/** Create the resources used in this view */
		virtual bool CreateResources(Device const& logical, Swapchain const& swapchain) = 0;
		/** Destroy the resources for this view. Called when they are no longer going to be used, or before we need to create them again */
		virtual void DestroyResources(Device const& logical) = 0;

		/** Reposition the part of the window to which this viewport should render */
		virtual void Reposition(glm::ivec2 newExtent, glm::ivec2 newOffset) = 0;

	protected:
		/** The position that this viewport will render to on the framebuffer */
		Geometry::ScreenRect rect;
	};
}
