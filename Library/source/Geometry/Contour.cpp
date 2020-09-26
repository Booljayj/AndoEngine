#include "Geometry/Equations.h"
#include "Geometry/Contour.h"
#include "Geometry/Curve.h"
#include "Geometry/LinearAlgebra.h"
#include "Utility/variant.hpp"

namespace Geometry {
	Rect Contour::Bounds() const {
		Rect total{};
		for (Curve const& curve : curves) {
			total.Encapsulate(curve.Bounds());
		}
		return total;
	}

	bool Contour::IsClosed() const {
		if (!curves.empty()) {
			glm::vec2 corner = curves.back().Position(1.0f);
			for (Curve const& curve : curves) {
				if (curve.Position(0.0f) != corner) return false;
				corner = curve.Position(1.0f);
			}
		}
		return true;
	}

	/** WTF is this? It's in msdfgen, but there's no indication what operation this actually represents. */
	float Shoelace(glm::vec2 const& a, glm::vec2 const& b) {
		return (b.x-a.x)*(a.y+b.y);
	}

	int8_t Contour::WindingSign() const {
		if (curves.empty()) return 0;
		float total = 0;
		if (curves.size() == 1) {
			glm::vec2 const a = curves[0].Position(0.0f);
			glm::vec2 const b = curves[0].Position(1.0f/3.0f);
			glm::vec2 const c = curves[0].Position(2.0f/3.0f);
			total += Shoelace(a, b);
			total += Shoelace(b, c);
			total += Shoelace(c, a);
		} else if (curves.size() == 2) {
			glm::vec2 const a = curves[0].Position(0.0f);
			glm::vec2 const b = curves[0].Position(0.5f);
			glm::vec2 const c = curves[1].Position(0.0f);
			glm::vec2 const d = curves[1].Position(0.5f);
			total += Shoelace(a, b);
			total += Shoelace(b, c);
			total += Shoelace(c, d);
			total += Shoelace(d, a);
		} else {
			glm::vec2 previous = curves.back().Position(0.0f);
			for (Curve const& curve : curves) {
				glm::vec2 current = curve.Position(0.0f);
				total += Shoelace(previous, current);
				previous = current;
			}
		}
		return glm::sign(total);
	}

	void ContourUtility::SplitContourIntoThirds(Contour& curve) {
		struct ThirdsVisitor {
			std::array<Curve, 3> thirds;
			void operator()(LinearCurve const& lc) {
				thirds[0] = LinearCurve{lc.p0, lc.Position(1.0f/3.0f), lc.colorChannels};
				thirds[1] = LinearCurve{lc.Position(1.0f/3.0f), lc.Position(2.0f/3.0f), lc.colorChannels};
				thirds[2] = LinearCurve{lc.Position(2.0f/3.0f), lc.p1, lc.colorChannels};
			};
			void operator()(QuadraticCurve const& qc) {
				thirds[0] = QuadraticCurve{qc.p0, glm::mix(qc.p0, qc.p1, 1.0f/3.0f), qc.Position(1.0f/3.0f), qc.colorChannels};
				thirds[1] = QuadraticCurve{qc.Position(1.0f/3.0f), glm::mix(glm::mix(qc.p0, qc.p1, 5.0f/9.0f), glm::mix(qc.p1, qc.p2, 4.0f/9.0f), 0.5f), qc.Position(2.0f/3.0f), qc.colorChannels};
				thirds[2] = QuadraticCurve{qc.Position(2.0f/3.0f), glm::mix(qc.p1, qc.p2, 2.0f/3.0f), qc.p2, qc.colorChannels};
			};
			void operator()( CubicCurve const& cc ) {
				thirds[0] = CubicCurve{cc.p0, glm::mix(cc.p0, cc.p1, 1.0f/3.0f), glm::mix(glm::mix(cc.p0, cc.p1, 1.0f/3.0f), glm::mix(cc.p1, cc.p2, 1.0f/3.0f), 1.0f/3.0f), cc.Position(1.0f/3.0f), cc.colorChannels};
				thirds[1] = CubicCurve{
					cc.Position(1.0f/3.0f),
					glm::mix(glm::mix(glm::mix(cc.p0, cc.p1, 1.0f/3.0f), glm::mix(cc.p1, cc.p2, 1.0f/3.0f), 1.0f/3.0f), glm::mix(glm::mix(cc.p1, cc.p2, 1.0f/3.0f), glm::mix(cc.p2, cc.p3, 1.0f/3.0f), 1.0f/3.0f), 2.0f/3.0f),
					glm::mix(glm::mix(glm::mix(cc.p0, cc.p1, 2.0f/3.0f), glm::mix(cc.p1, cc.p2, 2.0f/3.0f), 2.0f/3.0f), glm::mix(glm::mix(cc.p1, cc.p2, 2.0f/3.0f), glm::mix(cc.p2, cc.p3, 2.0f/3.0f), 2.0f/3.0f), 1.0f/3.0f),
					cc.Position(2.0f/3.0f),
					cc.colorChannels
				};
				thirds[2] = CubicCurve{cc.Position(2.0f/3.0f), glm::mix(glm::mix(cc.p1, cc.p2, 2.0f/3.0f), glm::mix(cc.p2, cc.p3, 2.0f/3.0f), 2.0f/3.0f), glm::mix(cc.p2, cc.p3, 2.0f/3.0f), cc.p3, cc.colorChannels};
			};
		};

		if (curve.curves.size() == 1) {
			ThirdsVisitor visitor;
			mpark::visit(visitor, curve.curves[0].variant);
			curve.curves.resize(3);
			curve.curves.insert(curve.curves.begin(), visitor.thirds.begin(), visitor.thirds.end());
		}
	}
}
