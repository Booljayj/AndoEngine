#pragma once
#include "Engine/StandardTypes.h"
#include "Geometry/CurveTypes.h"

namespace Geometry {
	/* A polymorphic line segment */
	struct Curve {
		std::variant<LinearCurve, QuadraticCurve, CubicCurve> variant;

		Curve();

		template<typename SegementType>
		Curve(SegementType const& segment)
		: variant(segment)
		{}

		template<typename SegementType>
		Curve& operator=(SegementType const& segment) {
			variant = segment;
			return *this;
		}

		/** Returns the position at the normalized distance along the curve */
		glm::vec2 Position(float alpha) const;
		/** Returns the direction at the normalized distance along the curve */
		glm::vec2 Direction(float alpha) const;
		/** Calculate the bounding box of the full edge segment */
		Rect Bounds() const;

		/** Returns the minimum signed distance between origin and the edge */
		CurveMinimumSignedDistance MinimumSignedDistance(glm::vec2 origin) const;

		/** Moves the start point of the edge segment */
		void SetStartPosition(glm::vec2 newStartPosition);
		/** Moves the end point of the edge segment */
		void SetEndPosition(glm::vec2 newEndPosition);

		/** Converts a previously retrieved signed distance from origin to pseudo-distance */
		SignedDistance DistanceToPseudoDistance(CurveMinimumSignedDistance const& distance, glm::vec2 origin) const;
	};
}