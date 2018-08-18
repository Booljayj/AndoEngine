#include <array>
#include "Geometry/Shape.h"
#include "Geometry/Contour.h"
#include "Geometry/Curve.h"
#include "Geometry/Rect.h"

namespace Geometry {
	bool Shape::IsClosed() const {
		for( Contour const& C : Contours ) {
			if( !C.IsClosed() ) return false;
		}
		return true;
	}

	Rect Shape::Bounds() const {
		Rect AABB{};
		for( Contour const& C : Contours ) {
			AABB.Encapsulate( C.Bounds() );
		}
		return AABB;
	}
}
