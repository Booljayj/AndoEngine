#include <glm/gtc/vec1.hpp>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>
#include "Geometry/Equations.h"
#include "Geometry/Contour.h"
#include "Geometry/Curve.h"
#include "Utility/variant.hpp"

namespace Geometry {
	Rect Contour::Bounds() const {
		Rect Total{};
		for( Curve const& C : Curves ) {
			Total.Encapsulate( C.Bounds() );
		}
		return Total;
	}

	bool Contour::IsClosed() const {
		if( !Curves.empty() ) {
			glm::vec2 Corner = Curves.back().Position( 1.0f );
			for( Curve const& C : Curves ) {
				if( C.Position( 0.0f ) != Corner ) return false;
				Corner = C.Position( 1.0f );
			}
		}
		return true;
	}

	/** WTF is this? It's in msdfgen, but there's no indication what operation this actually represents. */
	float Shoelace( glm::vec2 const& A, glm::vec2 const& B) {
		return (B.x-A.x)*(A.y+B.y);
	}

	int8_t Contour::WindingSign() const {
		if( Curves.empty() ) return 0;
		float Total = 0;
		if( Curves.size() == 1 ) {
			glm::vec2 const A = Curves[0].Position( 0.0f );
			glm::vec2 const B = Curves[0].Position( 1.0f/3.0f );
			glm::vec2 const C = Curves[0].Position( 2.0f/3.0f );
			Total += Shoelace( A, B );
			Total += Shoelace( B, C );
			Total += Shoelace( C, A );
		} else if( Curves.size() == 2 ) {
			glm::vec2 const A = Curves[0].Position( 0.0f );
			glm::vec2 const B = Curves[0].Position( 0.5f );
			glm::vec2 const C = Curves[1].Position( 0.0f );
			glm::vec2 const D = Curves[1].Position( 0.5f );
			Total += Shoelace( A, B );
			Total += Shoelace( B, C );
			Total += Shoelace( C, D );
			Total += Shoelace( D, A );
		} else {
			glm::vec2 Previous = Curves.back().Position( 0.0f );
			for( Curve const& C : Curves ) {
				glm::vec2 Current = C.Position( 0.0f );
				Total += Shoelace( Previous, Current );
				Previous = Current;
			}
		}
		return glm::sign( Total );
	}

	void ContourUtility::SplitContourIntoThirds( Contour& C ) {
		struct ThirdsVisitor {
			std::array<Curve, 3> Thirds;
			void operator()( LinearCurve const& LC ) {
				Thirds[0] = LinearCurve{ LC.P0, LC.Position( 1.0f/3.0f ), LC.ColorChannels };
				Thirds[1] = LinearCurve{ LC.Position( 1.0f/3.0f ), LC.Position( 2.0f/3.0f ), LC.ColorChannels };
				Thirds[2] = LinearCurve{ LC.Position( 2.0f/3.0f ), LC.P1, LC.ColorChannels };
			};
			void operator()( QuadraticCurve const& QC ) {
				Thirds[0] = QuadraticCurve{ QC.P0, glm::mix( QC.P0, QC.P1, 1.0f/3.0f ), QC.Position( 1.0f/3.0f ), QC.ColorChannels };
				Thirds[1] = QuadraticCurve{ QC.Position( 1.0f/3.0f ), glm::mix( glm::mix( QC.P0, QC.P1, 5.0f/9.0f ), glm::mix( QC.P1, QC.P2, 4.0f/9.0f ), 0.5f ), QC.Position( 2.0f/3.0f ), QC.ColorChannels };
				Thirds[2] = QuadraticCurve{ QC.Position( 2.0f/3.0f ), glm::mix( QC.P1, QC.P2, 2.0f/3.0f ), QC.P2, QC.ColorChannels };
			};
			void operator()( CubicCurve const& CC ) {
				Thirds[0] = CubicCurve{ CC.P0, glm::mix( CC.P0, CC.P1, 1.0f/3.0f ), glm::mix( glm::mix( CC.P0, CC.P1, 1.0f/3.0f ), glm::mix( CC.P1, CC.P2, 1.0f/3.0f ), 1.0f/3.0f ), CC.Position( 1.0f/3.0f ), CC.ColorChannels };
				Thirds[1] = CubicCurve{
					CC.Position( 1.0f/3.0f ),
					glm::mix( glm::mix( glm::mix( CC.P0, CC.P1, 1.0f/3.0f ), glm::mix( CC.P1, CC.P2, 1.0f/3.0f ), 1.0f/3.0f ), glm::mix( glm::mix( CC.P1, CC.P2, 1.0f/3.0f ), glm::mix( CC.P2, CC.P3, 1.0f/3.0f ), 1.0f/3.0f ), 2.0f/3.0f ),
					glm::mix( glm::mix( glm::mix( CC.P0, CC.P1, 2.0f/3.0f ), glm::mix( CC.P1, CC.P2, 2.0f/3.0f ), 2.0f/3.0f ), glm::mix( glm::mix( CC.P1, CC.P2, 2.0f/3.0f ), glm::mix( CC.P2, CC.P3, 2.0f/3.0f ), 2.0f/3.0f ), 1.0f/3.0f ),
					CC.Position( 2.0f/3.0f ),
					CC.ColorChannels
				};
				Thirds[2] = CubicCurve{ CC.Position( 2.0f/3.0f ), glm::mix( glm::mix( CC.P1, CC.P2, 2.0f/3.0f ), glm::mix( CC.P2, CC.P3, 2.0f/3.0f ), 2.0f/3.0f ), glm::mix( CC.P2, CC.P3, 2.0f/3.0f ), CC.P3, CC.ColorChannels };
			};
		};

		if( C.Curves.size() == 1 ) {
			ThirdsVisitor Visitor;
			mpark::visit( Visitor, C.Curves[0].Variant );
			C.Curves.resize( 3 );
			C.Curves.insert( C.Curves.begin(), Visitor.Thirds.begin(), Visitor.Thirds.end() );
		}
	}
}
