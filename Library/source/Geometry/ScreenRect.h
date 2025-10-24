#pragma once
#include "Engine/GLM.h"

namespace Geometry {
	/** Represents a bounding box that corresponds to some screen region, in pixel coordinates */
	struct ScreenRect {
		/** The offset to the starting corner of the rect, in pixel coordinates */
		glm::i32vec2 offset;
		/** The total extent of the rect, in pixels */
		glm::u32vec2 extent;
	};
}
