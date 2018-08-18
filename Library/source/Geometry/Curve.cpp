#include <glm/geometric.hpp>
#include <glm/gtx/exterior_product.hpp>
#include "Geometry/Curve.h"

namespace Geometry {
	Curve::Curve()
	: Variant( LinearCurve{ glm::vec2{ 0.0f, 0.0f }, glm::vec2{ 0.0f, 1.0f } } )
	{}

	glm::vec2 Curve::Position( float Alpha ) const {
		return mpark::visit( [&]( auto const& L ) { return L.Position( Alpha ); }, Variant );
	}
	glm::vec2 Curve::Direction( float Alpha ) const {
		return mpark::visit( [&]( auto const& L ) { return L.Direction( Alpha ); }, Variant );
	}
	Rect Curve::Bounds() const {
		return mpark::visit( [&]( auto const& L ) { return L.Bounds(); }, Variant );
	}

	CurveMinimumSignedDistance Curve::MinimumSignedDistance( glm::vec2 Origin ) const {
		return mpark::visit( [&]( auto const& L ) { return L.MinimumSignedDistance( Origin ); }, Variant );
	}

	void Curve::SetStartPosition( glm::vec2 NewStartPosition ) {
		return mpark::visit( [&]( auto& L ) { return L.SetStartPosition( NewStartPosition ); }, Variant );
	}
	void Curve::SetEndPosition( glm::vec2 NewEndPosition ) {
		return mpark::visit( [&]( auto& L ) { return L.SetEndPosition( NewEndPosition ); }, Variant );
	}

	SignedDistance Curve::DistanceToPseudoDistance( const CurveMinimumSignedDistance& Distance, glm::vec2 Origin ) const {
		if( Distance.NearestAlpha < 0.0f ) {
			glm::vec2 const Dir = glm::normalize( Direction( 0.0f ) );
			glm::vec2 const AQ = Origin - Position( 0.0f );
			float const ts = glm::dot(AQ, Dir);
			if( ts < 0.0f ) {
				float const PseudoDistance = glm::cross( AQ, Dir );
				if( fabs( PseudoDistance ) <= fabs( Distance.MinimumSignedDistance.Distance ) ) {
					return SignedDistance{ PseudoDistance, 0.0f };
				}
			}
		} else if ( Distance.NearestAlpha > 1.0f ) {
			glm::vec2 const Dir = glm::normalize( Direction( 1.0f ) );
			glm::vec2 const bq = Origin - Position( 1.0f );
			float const ts = glm::dot( bq, Dir );
			if( ts > 0.0f ) {
				float const PseudoDistance = glm::cross( bq, Dir );
				if( fabs( PseudoDistance ) <= fabs( Distance.MinimumSignedDistance.Distance ) ) {
					return SignedDistance{ PseudoDistance, 0.0f };
				}
			}
		}
		return Distance.MinimumSignedDistance;
	}
}
