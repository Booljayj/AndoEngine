#pragma once
#include "Engine/STL.h"
#include "Geometry/Rect.h"
#include "Geometry/ColorChannel.h"
#include "Geometry/LinearAlgebra.h"
#include "Geometry/SignedDistance.h"

namespace Geometry {
	struct CurveMinimumSignedDistance {
		SignedDistance minimumSignedDistance;
		float nearestAlpha;
	};

	/** A linear segment */
	struct LinearCurve {
		glm::vec2 p0 = glm::vec2{0.0f, 0.0f};
		glm::vec2 p1 = glm::vec2{0.0f, 0.0f};
		FColorChannel colorChannels = FColorChannel::White;

		LinearCurve() = default;
		LinearCurve(glm::vec2 const& inP0, glm::vec2 const& inP1, FColorChannel inColorChannels = FColorChannel::White);

		glm::vec2 Position(float alpha) const;
		glm::vec2 Direction(float alpha) const;
		Rect Bounds() const;
		CurveMinimumSignedDistance MinimumSignedDistance(glm::vec2 origin) const;

		void SetStartPosition(glm::vec2 newStartPosition);
		void SetEndPosition(glm::vec2 newEndPosition);
	};

	/** A quadratic Bezier curve segment */
	struct QuadraticCurve {
		glm::vec2 p0 = glm::vec2{0.0f, 0.0f};
		glm::vec2 p1 = glm::vec2{0.0f, 0.0f};
		glm::vec2 p2 = glm::vec2{0.0f, 0.0f};
		FColorChannel colorChannels = FColorChannel::White;

		QuadraticCurve();
		QuadraticCurve(glm::vec2 const& inP0, glm::vec2 const& inP1, glm::vec2 const& inP2, FColorChannel inColorChannels = FColorChannel::White);

		glm::vec2 Position(float alpha) const;
		glm::vec2 Direction(float alpha) const;
		Rect Bounds() const;
		CurveMinimumSignedDistance MinimumSignedDistance(glm::vec2 origin) const;

		void SetStartPosition(glm::vec2 newStartPosition);
		void SetEndPosition(glm::vec2 newEndPosition);
	};

	/* A cubic Bezier curve segment */
	struct CubicCurve {
		glm::vec2 p0 = glm::vec2{0.0f, 0.0f};
		glm::vec2 p1 = glm::vec2{0.0f, 0.0f};
		glm::vec2 p2 = glm::vec2{0.0f, 0.0f};
		glm::vec2 p3 = glm::vec2{0.0f, 0.0f};
		FColorChannel colorChannels = FColorChannel::White;

		CubicCurve();
		CubicCurve(glm::vec2 const& inP0, glm::vec2 const& inP1, glm::vec2 const& inP2, glm::vec2 const& inP3, FColorChannel inColorChannels = FColorChannel::White);

		glm::vec2 Position(float alpha) const;
		glm::vec2 Direction(float alpha) const;
		Rect Bounds() const;
		CurveMinimumSignedDistance MinimumSignedDistance(glm::vec2 origin) const;

		void SetStartPosition(glm::vec2 newStartPosition);
		void SetEndPosition(glm::vec2 newEndPosition);
	};
}
