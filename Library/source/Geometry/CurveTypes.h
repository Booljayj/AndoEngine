#pragma once
#include <vector>
#include <memory>
#include <glm/vec2.hpp>
#include "Geometry/Rect.h"
#include "Geometry/ColorChannel.h"
#include "Geometry/SignedDistance.h"

namespace Geometry
{
	struct CurveMinimumSignedDistance
	{
		SignedDistance MinimumSignedDistance;
		float NearestAlpha;
	};

	/** A linear segment */
	struct LinearCurve {
		glm::vec2 P0 = glm::vec2{ 0.0f, 0.0f };
		glm::vec2 P1 = glm::vec2{ 0.0f, 0.0f };
		FColorChannel ColorChannels = FColorChannel::White;

		LinearCurve() = default;
		LinearCurve( const glm::vec2& InP0, const glm::vec2& InP1, FColorChannel InColorChannels = FColorChannel::White );

		glm::vec2 Position( float Alpha ) const;
		glm::vec2 Direction( float Alpha ) const;
		Rect Bounds() const;
		CurveMinimumSignedDistance MinimumSignedDistance( glm::vec2 Origin ) const;

		void SetStartPosition( glm::vec2 NewStartPosition );
		void SetEndPosition( glm::vec2 NewEndPosition );
	};

	/** A quadratic Bezier curve segment */
	struct QuadraticCurve {
		glm::vec2 P0 = glm::vec2{ 0.0f, 0.0f };
		glm::vec2 P1 = glm::vec2{ 0.0f, 0.0f };
		glm::vec2 P2 = glm::vec2{ 0.0f, 0.0f };
		FColorChannel ColorChannels = FColorChannel::White;

		QuadraticCurve();
		QuadraticCurve( const glm::vec2& InP0, const glm::vec2& InP1, const glm::vec2& InP2, FColorChannel InColorChannels = FColorChannel::White );

		glm::vec2 Position( float Alpha ) const;
		glm::vec2 Direction( float Alpha ) const;
		Rect Bounds() const;
		CurveMinimumSignedDistance MinimumSignedDistance( glm::vec2 Origin ) const;

		void SetStartPosition( glm::vec2 NewStartPosition );
		void SetEndPosition( glm::vec2 NewEndPosition );
	};

	/* A cubic Bezier curve segment */
	struct CubicCurve {
		glm::vec2 P0 = glm::vec2{ 0.0f, 0.0f };
		glm::vec2 P1 = glm::vec2{ 0.0f, 0.0f };
		glm::vec2 P2 = glm::vec2{ 0.0f, 0.0f };
		glm::vec2 P3 = glm::vec2{ 0.0f, 0.0f };
		FColorChannel ColorChannels = FColorChannel::White;

		CubicCurve();
		CubicCurve( const glm::vec2& InP0, const glm::vec2& InP1, const glm::vec2& InP2, const glm::vec2& InP3, FColorChannel InColorChannels = FColorChannel::White );

		glm::vec2 Position( float Alpha ) const;
		glm::vec2 Direction( float Alpha ) const;
		Rect Bounds() const;
		CurveMinimumSignedDistance MinimumSignedDistance( glm::vec2 Origin ) const;

		void SetStartPosition( glm::vec2 NewStartPosition );
		void SetEndPosition( glm::vec2 NewEndPosition );
	};
}
