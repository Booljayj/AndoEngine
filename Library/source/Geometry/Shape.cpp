#include "Geometry/Shape.h"
#include "Geometry/Contour.h"
#include "Geometry/Curve.h"
#include "Geometry/Rect.h"

namespace Geometry {
	bool Shape::IsClosed() const {
		for (Contour const& contour : contours) {
			if (!contour.IsClosed()) return false;
		}
		return true;
	}

	Rect Shape::Bounds() const {
		Rect aabb{};
		for (Contour const& contour : contours) {
			aabb.Encapsulate(contour.Bounds());
		}
		return aabb;
	}
}
