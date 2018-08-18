#include <cmath>
#include <limits>
#include "Geometry/Equations.h"

namespace Geometry {
	int8_t SolveQuadratic( float* X, float A, float B, float C ) {
		if( fabs( A ) < std::numeric_limits<float>::min() ) {
			if( fabs( B ) < std::numeric_limits<float>::min() ) {
				if( C == 0.0f ) return -1;
				else return 0;
			} else {
				X[0] = -C/B;
				return 1;
			}
		}
		float Discriminant = (B*B)-(4.0f*A*C);
		if( Discriminant > 0 ) {
			Discriminant = sqrt( Discriminant );
			X[0] = ( -B + Discriminant )/(2.0f*A);
			X[1] = ( -B - Discriminant )/(2.0f*A);
			return 2;
		} else if( Discriminant == 0 ) {
			X[0] = B/(-2.0f*A);
			return 1;
		} else {
			return 0;
		}
	}

	int8_t SolveCubicNormed( float* X, float A, float B, float C ) {
		float const A2 = A*A;
		float Q = (A2-3.0f*B)/9.0f;
		float const R = (A*(2.0f*A2 - 9.0f*B)+27.0f*C)/54.0f;
		float const R2 = R*R;
		float const Q3 = Q*Q*Q;
		if( R2<Q3 ) {
			float T = R / sqrt( Q3 );
			if( T<-1.0f ) T = -1.0f;
			if( T>1.0f ) T = 1.0f;
			T = acos( T );
			A /= 3.0f;
			Q = -2.0f*sqrt( Q );
			X[0] = Q*cos( T/3.0f ) - A;
			X[1] = Q*cos( (T+2.0f*M_PI)/3.0f )-A;
			X[2] = Q*cos( (T-2.0f*M_PI)/3.0f )-A;
			return 3;
		} else {
			float AP = -pow( fabs( R )+sqrt( R2-Q3 ), 1.0f/3.0f );
			if( R < 0.0f ) AP = -AP;
			float const BP = (AP==0.0f) ? 0.0f : (Q/AP);
			A /= 3.0f;
			X[0] = (AP+BP)-A;
			X[1] = -0.5f*(AP+BP)-A;
			X[2] = 0.5f*sqrt( 3.0f )*(AP-BP);
			if( fabs( X[2] ) < std::numeric_limits<float>::min() ) return 2;
			else return 1;
		}
	}

	int8_t SolveCubic( float* X, float A, float B, float C, float D ) {
		if( fabs( A ) < std::numeric_limits<float>::min() ) return SolveQuadratic( X, B, C, D );
		return SolveCubicNormed( X, B/A, C/A, D/A );
	}
}
