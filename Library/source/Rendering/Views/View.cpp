#include "Rendering/Views/View.h"

namespace Rendering {
	ViewParameters::ViewParameters(View const& view, glm::u32vec2 render_target_extent)
		//@todo When this is a weak pointer, we'll pin it here to keep the registry alive while culling and recording.
		: registry(view.registry)
		, rect(view.rect_calculator ? view.rect_calculator->Calculate(render_target_extent) : ViewRect{ render_target_extent })
		, camera(view.camera_calculator ? view.camera_calculator->Calculate(*registry, rect.extent) : ViewCamera{})
		, num_culling_threads(view.num_culling_threads)
		, num_recording_threads(view.num_recording_threads)
	{}
}
