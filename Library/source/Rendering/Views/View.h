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

		/** The id of this view. Can be used to distinguish it from other views. */
		StringID id;

		//@todo This needs to be a shared pointer to some type of "world" struct, which contains the registry.
		entt::registry const* registry = nullptr;

		/** calculator struct used to determine the view rect for this view */
		std::unique_ptr<ViewRectCalculator> rect_calculator;
		/** calculator struct used to determine the view camera for this view */
		std::unique_ptr<ViewCameraCalculator> camera_calculator;

		/** The number of threads that should be used when culling geomtry for thie view. Views with many entities should use multiple threads, but simpler views can use fewer threads to save on resources. */
		uint16_t num_culling_threads = 1;
		/** The number of threads that should be used when recording rendering commands for this view. Views with many entities should use multiple threads, but simpler views can use fewer threads to save on resources. */
		uint16_t num_recording_threads = 1;
	};

	/** Gathered parameters for a view that will be used by worker threads */
	struct ViewParameters {
		ViewParameters(View const& view, glm::u32vec2 render_target_extent);
		ViewParameters(ViewParameters const&) = delete;
		ViewParameters(ViewParameters&&) = default;

		entt::registry const* registry = nullptr;

		ViewRect rect;
		ViewCamera camera;

		/** The number of threads to use when culling geometry */
		uint16_t num_culling_threads = 1;
		/** The number of threads that will be used to render this view */
		uint16_t num_recording_threads = 1;

		uint32_t globals_index_offset = 0;
		uint32_t objects_index_offset = 0;
	};
}
