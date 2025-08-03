#pragma once
#include "Engine/Core.h"

namespace Geometry {
	/** Solve the equation in the form Ax^2+Bx+c=0, returns the number of solutions from 0-2 */
	int8_t SolveQuadratic(float* x, float a, float b, float c);
	/** Solve the equation in the form Ax^3+Bx^2+Cx+D=0, returns the number of solutions from 0-3 */
	int8_t SolveCubic(float* x, float a, float b, float c, float d);
}
