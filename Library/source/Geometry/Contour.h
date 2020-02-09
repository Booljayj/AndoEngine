#pragma once
#include <vector>
#include "Geometry/Rect.h"

namespace Geometry {
	struct Curve;

	/** Represents a continuous directional path */
	struct Contour {
		std::vector<Curve> curves;

		/** Calculates the axially-oriented bounding box of the contour */
		Rect Bounds() const;
		/** True if the curves in this contour are continuous and the end point is the same as the start point */
		bool IsClosed() const;
		/** Computes the winding sign of the contour. Returns 1 if positive, -1 if negative. */
		int8_t WindingSign() const;
	};

	namespace ContourUtility {
		/** Splits a single-curve contour into thirds which together represent the original contour */
		void SplitContourIntoThirds(Contour& contour);
	}
}
