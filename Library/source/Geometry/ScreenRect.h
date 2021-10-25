#pragma once
#include "Engine/STL.h"
#include "Geometry/GLM.h"

namespace Geometry {
	/** Represents a bounding box that corresponds to some screen region */
	struct ScreenRect {
		glm::i32vec2 offset;
		glm::u32vec2 extent;
	};
}
