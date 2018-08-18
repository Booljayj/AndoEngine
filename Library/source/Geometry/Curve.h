#include <array>
#include <glm/vec2.hpp>
#include "Geometry/CurveTypes.h"
#include "Utility/variant.hpp"

namespace Geometry {
	/* A polymorphic line segment */
	struct Curve {
		mpark::variant<LinearCurve, QuadraticCurve, CubicCurve> Variant;

		Curve();

		template<typename TSEGMENT>
		Curve( const TSEGMENT& Segment )
		: Variant( Segment )
		{}

		template<typename TSEGMENT>
		Curve& operator=( const TSEGMENT& Segment ) {
			Variant = Segment;
			return *this;
		}

		/** Returns the position at the normalized distance along the curve */
		glm::vec2 Position( float Alpha ) const;
		/** Returns the direction at the normalized distance along the curve */
		glm::vec2 Direction( float Alpha ) const;
		/** Calculate the bounding box of the full edge segment */
		Rect Bounds() const;

		/** Returns the minimum signed distance between origin and the edge */
		CurveMinimumSignedDistance MinimumSignedDistance( glm::vec2 Origin ) const;

		/** Moves the start point of the edge segment */
		void SetStartPosition( glm::vec2 NewStartPosition );
		/** Moves the end point of the edge segment */
		void SetEndPosition( glm::vec2 NewEndPosition );

		/** Converts a previously retrieved signed distance from origin to pseudo-distance */
		SignedDistance DistanceToPseudoDistance( const CurveMinimumSignedDistance& Distance, glm::vec2 Origin ) const;
	};
}