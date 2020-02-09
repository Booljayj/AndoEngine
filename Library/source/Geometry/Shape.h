#pragma once
#include <vector>
#include <memory>

namespace Geometry {
	struct Contour;
	struct Rect;

	/** Represents a 2D shape composed of several contours */
	struct Shape {
		std::vector<Contour> contours;

		/** true if all the contours in this shape are closed */
		bool IsClosed() const;
		/* Computes the shape's bounding box */
		Rect Bounds() const;
	};
}
