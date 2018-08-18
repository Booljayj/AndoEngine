#include <array>
#include <glm/gtx/exterior_product.hpp>
#include <glm/geometric.hpp>
#include "Geometry/Equations.h"
#include "Geometry/CurveTypes.h"

namespace Geometry {
	LinearCurve::LinearCurve( glm::vec2 const& InP0, glm::vec2 const& InP1, FColorChannel InColorChannels )
	: P0( InP0 ), P1( InP1 )
	, ColorChannels( InColorChannels )
	{}

	QuadraticCurve::QuadraticCurve( glm::vec2 const& InP0, glm::vec2 const& InP1, glm::vec2 const& InP2, FColorChannel InColorChannels  )
	: P0( InP0 ), P1( InP1 ), P2( InP2 )
	, ColorChannels( InColorChannels )
	{}

	CubicCurve::CubicCurve( glm::vec2 const& InP0, glm::vec2 const& InP1, glm::vec2 const& InP2, glm::vec2 const& InP3, FColorChannel InColorChannels )
	: P0( InP0 ), P1( InP1 ), P2( InP2 ), P3( InP3 )
	, ColorChannels( InColorChannels )
	{}

	glm::vec2 LinearCurve::Position( float Alpha ) const {
		return glm::mix( P0, P1, Alpha );
	}
	glm::vec2 QuadraticCurve::Position( float Alpha ) const {
		return glm::mix( glm::mix( P0, P1, Alpha ), glm::mix( P1, P2, Alpha ), Alpha );
	}
	glm::vec2 CubicCurve::Position( float Alpha ) const {
		glm::vec2 const P12 = glm::mix( P1, P2, Alpha );
		return glm::mix( glm::mix( glm::mix( P0, P1, Alpha ), P12, Alpha ), glm::mix( P12, glm::mix( P2, P3, Alpha ), Alpha ), Alpha );
	}

	glm::vec2 LinearCurve::Direction( float Alpha ) const {
		return P1 - P0;
	}
	glm::vec2 QuadraticCurve::Direction( float Alpha ) const {
		return glm::mix( P1 - P0, P2 - P1, Alpha );
	}
	glm::vec2 CubicCurve::Direction( float Alpha ) const {
		if( Alpha == 0 ) return P2 - P0;
		if( Alpha == 1 ) return P3 - P1;
		return glm::mix( glm::mix( P1 - P0, P2 - P1, Alpha ), glm::mix( P2 - P1, P3 - P2, Alpha ), Alpha );
	}

	Rect LinearCurve::Bounds() const {
		Rect AABB{ P0 };
		AABB.Encapsulate( P1 );
		return AABB;
	}
	Rect QuadraticCurve::Bounds() const {
		Rect AABB{ P0 };
		AABB.Encapsulate( P2 );
		glm::vec2 const Bottom = ( P1 - P0 ) - ( P2 - P1 );
		if( Bottom.x != 0.0f ) {
			float const Alpha = ( P1.x - P0.x ) / Bottom.x;
			if( Alpha > 0 && Alpha < 1 ) AABB.Encapsulate( Position( Alpha ) );
		}
		if( Bottom.y != 0.0f ) {
			float const Alpha = ( P1.y - P0.y ) / Bottom.y;
			if( Alpha > 0 && Alpha < 1 ) AABB.Encapsulate( Position( Alpha ) );
		}
		return AABB;
	}
	Rect CubicCurve::Bounds() const {
		Rect AABB{ P0 };
		AABB.Encapsulate( P3 );
		glm::vec2 const A0 = P1 - P0;
		glm::vec2 const A1 = 2.0f * ( P2 - P1 - A0 );
		glm::vec2 const A2 = P3- ( 3.0f * P2 ) + ( 3.0f * P1 ) - P0;
		{
			float Alphas[2];
			size_t const Solutions = SolveQuadratic( Alphas, A2.x, A1.x, A0.x );
			for( size_t Index = 0; Index < Solutions; ++Index ) {
				if( Alphas[Index] > 0 && Alphas[Index] < 1 ) AABB.Encapsulate( Position( Alphas[Index] ) );
			}
		}
		{
			float Alphas[2];
			size_t const Solutions = SolveQuadratic( Alphas, A2.y, A1.y, A0.y );
			for( size_t Index = 0; Index < Solutions; ++Index ) {
				if( Alphas[Index] > 0 && Alphas[Index] < 1 ) AABB.Encapsulate( Position( Alphas[Index] ) );
			}
		}
		return AABB;
	}

	template<typename T>
	uint8_t NonZeroSign( T Alpha ) {
		return 2 * ( Alpha > T{0} ) - 1;
	}

	CurveMinimumSignedDistance LinearCurve::MinimumSignedDistance( glm::vec2 Origin ) const {
		SignedDistance MinimumSignedDistance;
		glm::vec2 const AQ = Origin - P0;
		glm::vec2 const AB = P1 - P0;
		float NearestAlpha = glm::dot( AQ, AB ) / glm::dot( AB, AB );
		glm::vec2 const EQ = ( NearestAlpha > 0.5f ? P1 : P0 ) - Origin;
		float const EndpointDistance = glm::length( EQ );
		if( NearestAlpha > 0.0f && NearestAlpha < 1.0f ) {
			float const ABLength = glm::length( AB );
			glm::vec2 const ABPerpendicular{ AB.y/ABLength, -AB.x/ABLength };
			float const OrthoDistance = glm::dot( ABPerpendicular, AQ );
			if( fabs( OrthoDistance ) < EndpointDistance ) MinimumSignedDistance = SignedDistance{ OrthoDistance, 0 };
		} else {
			MinimumSignedDistance = SignedDistance{
				NonZeroSign( glm::cross( AQ, AB ) ) * EndpointDistance,
				fabs( glm::dot( glm::normalize( AB ), glm::normalize( EQ ) ) )
			};
		}
		return CurveMinimumSignedDistance{ MinimumSignedDistance, NearestAlpha };
	}
	CurveMinimumSignedDistance QuadraticCurve::MinimumSignedDistance( glm::vec2 Origin ) const {
		glm::vec2 const QA = P0 - Origin;
		glm::vec2 const AB = P1 - P0;
		glm::vec2 const BR = P0 + P2 - P1 - P1;
		float const A = glm::dot( BR, BR );
		float const B = 3.0f * glm::dot( AB, BR );
		float const C = 2.0f * glm::dot( AB, AB ) + glm::dot( QA, BR );
		float const D = glm::dot( QA, AB );
		float Alphas[3];
		uint8_t const Solutions = SolveCubic( Alphas, A, B, C, D );

		float MinimumDistance = NonZeroSign( glm::cross( AB, QA ) ) * glm::length( QA ); // Distance from A
		float NearestAlpha = -glm::dot( QA, AB ) / glm::dot( AB, AB );
		{
			float const Distance = NonZeroSign( glm::cross( P2 - P1, P2 - Origin ) ) * glm::length( P2 - Origin ); // Distance from B
			if( fabs( Distance ) < fabs( MinimumDistance ) ) {
				MinimumDistance = Distance;
				NearestAlpha = glm::dot( Origin - P1, P2 - P1 ) / glm::dot( P2 - P1, P2 - P1 );
			}
		}
		for( size_t Index = 0; Index < Solutions; ++Index ) {
			if( Alphas[Index] > 0.0f && Alphas[Index] < 1.0f ) {
				glm::vec2 const Endpoint = P0 + ( 2.0f * Alphas[Index] * AB ) + ( Alphas[Index] * Alphas[Index] * BR );
				float const Distance = NonZeroSign( glm::cross( P2 - P0, Endpoint - Origin ) ) * glm::length( Endpoint - Origin );
				if( fabs( Distance ) <= fabs( MinimumDistance ) ) {
					MinimumDistance = Distance;
					NearestAlpha = Alphas[Index];
				}
			}
		}

		if( NearestAlpha >= 0.0f && NearestAlpha <= 1.0f ) {
			return CurveMinimumSignedDistance{ SignedDistance{ MinimumDistance, 0.0f }, NearestAlpha };
		} if( NearestAlpha < 0.5f ) {
			return CurveMinimumSignedDistance{ SignedDistance{ MinimumDistance, fabs( glm::dot( glm::normalize( AB ), glm::normalize( QA ) ) ) }, NearestAlpha };
		} else {
			return CurveMinimumSignedDistance{ SignedDistance{ MinimumDistance, fabs( glm::dot( glm::normalize( P2 - P1 ), glm::normalize( P2 - Origin ) ) ) }, NearestAlpha };
		}
	}
	CurveMinimumSignedDistance CubicCurve::MinimumSignedDistance( glm::vec2 Origin ) const {
		glm::vec2 const QA = P0 - Origin;
		glm::vec2 const AB = P1 - P0;
		glm::vec2 const BR = P2 - P1 - AB;
		glm::vec2 const AS = ( P3 - P2 ) - ( P2 - P1 ) - BR;

		glm::vec2 EPDirection = Direction( 0.0f );
		float MinimumDistance = NonZeroSign( glm::cross( EPDirection, QA ) ) * glm::length( QA ); // Distance from A
		float NearestAlpha = -glm::dot( QA, EPDirection ) / glm::dot( EPDirection, EPDirection );
		{
			EPDirection = Direction( 1.0f );
			float const Distance = NonZeroSign( glm::cross( EPDirection, P3 - Origin ) ) * glm::length( P3 - Origin ); // Distance from B
			if( fabs( Distance) < fabs( MinimumDistance ) ) {
				MinimumDistance = Distance;
				NearestAlpha = glm::dot( Origin + EPDirection - P3, EPDirection ) / glm::dot( EPDirection, EPDirection );
			}
		}
		// Iterative minimum Distance search
		static constexpr size_t CUBIC_SEARCH_STARTS = 10;
		static constexpr size_t CUBIC_SEARCH_STEPS = 10;
		for( size_t Index = 0; Index <= CUBIC_SEARCH_STARTS; ++Index ) {
			float Alpha = (float)Index / (float)CUBIC_SEARCH_STARTS;
			for( size_t Step = 0;; ++Step ) {
				glm::vec2 const QPT = Position( Alpha ) - Origin;
				float const Distance = NonZeroSign( glm::cross( Direction( Alpha ), QPT ) ) * glm::length( QPT );
				if( fabs( Distance ) < fabs( MinimumDistance ) ) {
					MinimumDistance = Distance;
					NearestAlpha = Alpha;
				}

				if( Step == CUBIC_SEARCH_STEPS ) break;

				// Improve Alphas
				glm::vec2 const D1 = ( 3.0f * AS * Alpha * Alpha ) + ( 6.0f * BR * Alpha ) + ( 3.0f * AB );
				glm::vec2 const D2 = ( 6.0f * AS * Alpha ) + ( 6.0f * BR );
				Alpha -= glm::dot( QPT, D1 ) / ( glm::dot( D1, D1 ) + glm::dot( QPT, D2 ) );
				if( Alpha < 0.0f || Alpha > 1.0f ) break;
			}
		}

		if( NearestAlpha >= 0.0f && NearestAlpha <= 1.0f )
			return CurveMinimumSignedDistance{ SignedDistance{ MinimumDistance, 0.0f }, NearestAlpha };
		if( NearestAlpha < 0.5f )
			return CurveMinimumSignedDistance{ SignedDistance{ MinimumDistance, fabs( glm::dot( glm::normalize( Direction( 0.0f ) ), glm::normalize( QA ) ) ) }, NearestAlpha };
		else
			return CurveMinimumSignedDistance{ SignedDistance{ MinimumDistance, fabs( glm::dot( glm::normalize( Direction( 1.0f ) ), glm::normalize( P3 - Origin ) ) ) }, NearestAlpha };
	}

	void LinearCurve::SetStartPosition( glm::vec2 NewStartPosition ) {
		P0 = NewStartPosition;
	}
	void QuadraticCurve::SetStartPosition( glm::vec2 NewStartPosition ) {
		glm::vec2 const OriginalP1 = P1;
		glm::vec2 const OriginalP01 = P0 - P1;
		glm::vec2 const OriginalP21 = P2 - P1;
		P1 += glm::cross( OriginalP01, NewStartPosition - P0 ) / glm::cross( OriginalP01, OriginalP21 ) * OriginalP21;
		P0 = NewStartPosition;
		if( glm::dot( OriginalP01, P0 - P1 ) < 0.0f ) P1 = OriginalP1;
	}
	void CubicCurve::SetStartPosition( glm::vec2 NewStartPosition ) {
		P1 += ( NewStartPosition - P0 );
		P0 = NewStartPosition;
	}

	void LinearCurve::SetEndPosition( glm::vec2 NewEndPosition ) {
		P1 = NewEndPosition;
	}
	void QuadraticCurve::SetEndPosition( glm::vec2 NewEndPosition ) {
		glm::vec2 const OriginalP1 = P1;
		glm::vec2 const OriginalP01 = P0 - P1;
		glm::vec2 const OriginalP21 = P2 - P1;
		P1 += glm::cross( OriginalP21, NewEndPosition - P2 ) / glm::cross( OriginalP21, OriginalP01 ) * OriginalP01;
		P2 = NewEndPosition;
		if( glm::dot( OriginalP21, P2 - P1 ) < 0.0f ) P1 = OriginalP1;
	}
	void CubicCurve::SetEndPosition( glm::vec2 NewEndPosition ) {
		P2 += ( NewEndPosition - P3 );
		P3 = NewEndPosition;
	}
}