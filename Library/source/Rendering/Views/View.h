#pragma once
#include "Engine/STL.h"
#include "EntityFramework/EntityRegistry.h"
#include "EntityFramework/EntityTypes.h"
#include "Geometry/GLM.h"
#include "Geometry/ScreenRect.h"
#include "Rendering/MaterialComponent.h"
#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Rendering/Vulkan/VulkanResourcesHelpers.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

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
		virtual EPreparationResult Prepare(EntityRegistry const& registry, FrameResources& frame) const = 0;
		/** Record rendering commands for this view for a single frame */
		virtual void Record(const EntityRegistry& registry, const FrameResources& frame, size_t index) const = 0;

		/** Create the resources used in this view */
		virtual bool CreateResources(VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain) = 0;
		/** Destroy the resources for this view. Called when they are no longer going to be used, or before we need to create them again */
		virtual void DestroyResources(VulkanLogicalDevice const& logical);

		/** Reposition the part of the window to which this viewport should render */
		virtual void Reposition(glm::ivec2 newExtent, glm::ivec2 newOffset) = 0;

	protected:
		/** The position that this viewport will render to on the framebuffer */
		Geometry::ScreenRect rect;
	};
}
