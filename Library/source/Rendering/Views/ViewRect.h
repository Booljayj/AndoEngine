#pragma once
#include "Engine/GLM.h"

namespace Rendering {
	/** A rect that defines the draw area for a view */
	struct ViewRect {
		glm::i32vec2 offset;
		glm::u32vec2 extent;
	};

	/** Base class for  */
	struct ViewRectCalculator {
		virtual ~ViewRectCalculator() = default;
		virtual ViewRect Calculate(glm::u32vec2 surface_extent) const = 0;
	};

	struct FullViewRectCalculator : public ViewRectCalculator {
		virtual ViewRect Calculate(glm::u32vec2 surface_extent) const override;
	};

	struct FixedViewRectCalculator : public ViewRectCalculator {
		glm::i32vec2 offset;
		glm::u32vec2 extent;

		virtual ViewRect Calculate(glm::u32vec2 surface_extent) const override;
	};

	struct RelativeViewRectCalculator : public ViewRectCalculator {
		glm::vec2 relative_offset;
		glm::vec2 relative_extent;

		virtual ViewRect Calculate(glm::u32vec2 surface_extent) const override;
	};
}
