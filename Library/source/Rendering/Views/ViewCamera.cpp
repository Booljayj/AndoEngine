#include "Rendering/Views/ViewCamera.h"
#include "ViewCamera.h"

namespace Rendering {
	ViewCamera EditorViewCameraCalculator::Calculate(entt::registry const& registry, glm::uvec2 draw_size) const {
		ViewCamera result;
		result.transform = glm::identity<glm::mat4>();
		result.aspect = static_cast<float>(draw_size.x) / static_cast<float>(draw_size.y);

		return result;
	}
}
