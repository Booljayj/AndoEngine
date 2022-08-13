#pragma once
#include "Engine/StandardTypes.h"

namespace Geometry {
	/** Represents a bounding box that corresponds to some screen region */
	struct ScreenRect {
		glm::i32vec2 offset;
		glm::u32vec2 extent;
	};
}
