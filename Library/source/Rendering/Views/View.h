#pragma once
#include "Engine/Core.h"
#include "Engine/GLM.h"
#include "Engine/StringID.h"
#include "Rendering/Views/ViewCamera.h"
#include "Rendering/Views/ViewRect.h"

namespace Rendering {
	/** A view defines a region within a surface where rendering will take place, and what will be rendered there. */
	struct View {
		View(StringID id) : id(id) {}
		View(View const&) = delete;
		View(View&&) = default;
		virtual ~View() = default;

		/** The id of this view. Can be used to distinguish it from other views. */
		StringID id;

		/** The number of threads that should be used when recording rendering commands for this view. Views with many entities should use multiple threads, but simpler views can use fewer threads to save on resources. */
		uint16_t num_threads = 1;

		/** calculator struct used to determine the view rect for this view */
		std::unique_ptr<ViewRectCalculator> rect_calculator;
		/** calculator struct used to determine the view camera for this view */
		std::unique_ptr<ViewCameraCalculator> camera_calculator;
	};

	/** Gathered parameters that will be used to render a view */
	struct ViewRenderingParameters {
		ViewRect rect;
		ViewCamera camera;

		/** The frustum for this view */
		glm::mat4 frustum;

		/** The number of threads that will be used to render this view */
		size_t num_threads = 1;
		/** The set of non-culled entities that should be rendered to this view */
		std::vector<entt::entity> entities;
	};
}
