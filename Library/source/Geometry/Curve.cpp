#include <glm/geometric.hpp>
#include <glm/gtx/exterior_product.hpp>
#include "Geometry/Curve.h"

namespace Geometry {
	Curve::Curve()
	: variant(LinearCurve{glm::vec2{0.0f, 0.0f}, glm::vec2{0.0f, 1.0f}})
	{}

	glm::vec2 Curve::Position(float alpha) const {
		return std::visit([&]( auto const& segment) { return segment.Position(alpha); }, variant);
	}
	glm::vec2 Curve::Direction(float alpha) const {
		return std::visit([&](auto const& segment) { return segment.Direction(alpha); }, variant);
	}
	Rect Curve::Bounds() const {
		return std::visit([&](auto const& segment) { return segment.Bounds(); }, variant);
	}

	CurveMinimumSignedDistance Curve::MinimumSignedDistance(glm::vec2 origin) const {
		return std::visit([&](auto const& segment) { return segment.MinimumSignedDistance(origin); }, variant);
	}

	void Curve::SetStartPosition(glm::vec2 newStartPosition) {
		return std::visit([&](auto& segment) { return segment.SetStartPosition(newStartPosition); }, variant);
	}
	void Curve::SetEndPosition(glm::vec2 newEndPosition) {
		return std::visit([&](auto& segment) { return segment.SetEndPosition(newEndPosition); }, variant);
	}

	SignedDistance Curve::DistanceToPseudoDistance(CurveMinimumSignedDistance const& distance, glm::vec2 origin) const {
		if (distance.nearestAlpha < 0.0f) {
			glm::vec2 const dir = glm::normalize(Direction(0.0f));
			glm::vec2 const aq = origin - Position(0.0f);
			float const ts = glm::dot(aq, dir);
			if (ts < 0.0f) {
				float const pseudoDistance = glm::cross(aq, dir);
				if (fabs(pseudoDistance) <= fabs(distance.minimumSignedDistance.distance)) {
					return SignedDistance{pseudoDistance, 0.0f};
				}
			}
		} else if (distance.nearestAlpha > 1.0f) {
			glm::vec2 const dir = glm::normalize(Direction(1.0f));
			glm::vec2 const bq = origin - Position(1.0f);
			float const ts = glm::dot(bq, dir);
			if (ts > 0.0f) {
				float const pseudoDistance = glm::cross(bq, dir);
				if (fabs(pseudoDistance) <= fabs(distance.minimumSignedDistance.distance)) {
					return SignedDistance{pseudoDistance, 0.0f};
				}
			}
		}
		return distance.minimumSignedDistance;
	}
}
