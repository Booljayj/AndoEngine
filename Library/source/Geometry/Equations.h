#pragma once
#include <cstdint>

namespace Geometry {
	/** Solve the equation in the form Ax^2+Bx+C=0, returns the number of solutions from 0-2 */
	int8_t SolveQuadratic( float* X, float A, float B, float C );
	/** Solve the equation in the form Ax^3+Bx^2+Cx+D=0, returns the number of solutions from 0-3 */
	int8_t SolveCubic( float* X, float A, float B, float C, float D );
}
