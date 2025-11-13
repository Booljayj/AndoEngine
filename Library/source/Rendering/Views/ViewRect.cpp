#include "Rendering/Views/ViewRect.h"

namespace Rendering {
	ViewRect FullViewRectCalculator::Calculate(glm::u32vec2 surface_extent) const {
		return ViewRect{ surface_extent };
	}
	
	ViewRect FixedViewRectCalculator::Calculate(glm::u32vec2 surface_extent) const {
		//@todo we need to clamp the fixed offset and extent so they are within the surface extent.
		glm::i64vec2 min{ offset.x, offset.y };
		glm::i64vec2 max{ offset.x + extent.x, offset.y + extent.y };

		min = glm::max(min, glm::zero<glm::i64vec2>());
		max = glm::min(max, { surface_extent.x, surface_extent.y });

		return ViewRect{
			glm::i32vec2{ static_cast<int32_t>(min.x), static_cast<int32_t>(min.y) },
			glm::u32vec2{ static_cast<uint32_t>(max.x - min.x), static_cast<uint32_t>(max.y - min.y) }
		};
	}

	ViewRect RelativeViewRectCalculator::Calculate(glm::u32vec2 surface_extent) const {
		return ViewRect{
			glm::i32vec2{ static_cast<int32_t>(relative_offset.x * surface_extent.x), static_cast<int32_t>(relative_offset.y * surface_extent.y) },
			glm::u32vec2{ static_cast<uint32_t>(relative_extent.x * surface_extent.x), static_cast<uint32_t>(relative_extent.y * surface_extent.y) }
		};
	}
}
