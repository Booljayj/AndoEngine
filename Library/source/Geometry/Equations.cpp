#include "Geometry/Equations.h"

namespace Geometry {
	int8_t SolveQuadratic(float* x, float a, float b, float c) {
		if (fabs(a) < std::numeric_limits<float>::min()) {
			if (fabs(b) < std::numeric_limits<float>::min()) {
				if (c == 0.0f) return -1;
				else return 0;
			} else {
				x[0] = -c/b;
				return 1;
			}
		}
		float discriminant = (b*b)-(4.0f*a*c);
		if (discriminant > 0) {
			discriminant = sqrt(discriminant);
			x[0] = (-b + discriminant) / (2.0f*a);
			x[1] = (-b - discriminant) / (2.0f*a);
			return 2;
		} else if (discriminant == 0) {
			x[0] = b/(-2.0f*a);
			return 1;
		} else {
			return 0;
		}
	}

	int8_t SolveCubicNormed(float* x, float a, float b, float c) {
		float const a2 = a*a;
		float q = (a2-3.0f*b)/9.0f;
		float const r = (a*(2.0f*a2 - 9.0f*b)+27.0f*c)/54.0f;
		float const r2 = r*r;
		float const q3 = q*q*q;
		if (r2 < q3) {
			float t = r / sqrt(q3);
			if (t < -1.0f) t = -1.0f;
			if (t > 1.0f) t = 1.0f;
			t = acos(t);
			a /= 3.0f;
			q = -2.0f * sqrt(q);
			x[0] = q * cos(t/3.0f) - a;
			x[1] = q * cos((t+2.0f*M_PI)/3.0f )-a;
			x[2] = q * cos((t-2.0f*M_PI)/3.0f )-a;
			return 3;
		} else {
			float ap = -pow(fabs(r) + sqrt(r2-q3), 1.0f/3.0f);
			if (r < 0.0f) ap = -ap;
			float const bp = (ap==0.0f) ? 0.0f : (q/ap);
			a /= 3.0f;
			x[0] = (ap+bp)-a;
			x[1] = -0.5f*(ap+bp)-a;
			x[2] = 0.5f*sqrt(3.0f)*(ap-bp);
			if (fabs(x[2]) < std::numeric_limits<float>::min()) return 2;
			else return 1;
		}
	}

	int8_t SolveCubic(float* x, float a, float b, float c, float d) {
		if (fabs(a) < std::numeric_limits<float>::min()) return SolveQuadratic(x, b, c, d);
		return SolveCubicNormed(x, b/a, c/a, d/a);
	}
}
