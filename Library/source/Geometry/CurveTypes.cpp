#include <array>
#include <glm/gtx/exterior_product.hpp>
#include <glm/geometric.hpp>
#include "Geometry/Equations.h"
#include "Geometry/CurveTypes.h"

namespace Geometry {
	LinearCurve::LinearCurve(glm::vec2 const& inP0, glm::vec2 const& inP1, FColorChannel inColorChannels)
	: p0(inP0), p1(inP1)
	, colorChannels(inColorChannels)
	{}

	QuadraticCurve::QuadraticCurve(glm::vec2 const& inP0, glm::vec2 const& inP1, glm::vec2 const& inP2, FColorChannel inColorChannels)
	: p0(inP0), p1(inP1), p2(inP2)
	, colorChannels(inColorChannels)
	{}

	CubicCurve::CubicCurve( glm::vec2 const& inP0, glm::vec2 const& inP1, glm::vec2 const& inP2, glm::vec2 const& inP3, FColorChannel inColorChannels)
	: p0(inP0), p1(inP1), p2(inP2), p3(inP3)
	, colorChannels(inColorChannels)
	{}

	glm::vec2 LinearCurve::Position(float alpha) const {
		return glm::mix(p0, p1, alpha);
	}
	glm::vec2 QuadraticCurve::Position(float alpha) const {
		return glm::mix(glm::mix(p0, p1, alpha), glm::mix(p1, p2, alpha), alpha);
	}
	glm::vec2 CubicCurve::Position(float alpha) const {
		glm::vec2 const p12 = glm::mix(p1, p2, alpha);
		return glm::mix(glm::mix( glm::mix(p0, p1, alpha), p12, alpha), glm::mix(p12, glm::mix(p2, p3, alpha), alpha), alpha);
	}

	glm::vec2 LinearCurve::Direction(float alpha) const {
		return p1 - p0;
	}
	glm::vec2 QuadraticCurve::Direction(float alpha) const {
		return glm::mix(p1 - p0, p2 - p1, alpha);
	}
	glm::vec2 CubicCurve::Direction(float alpha) const {
		if (alpha == 0) return p2 - p0;
		if (alpha == 1) return p3 - p1;
		return glm::mix(glm::mix(p1 - p0, p2 - p1, alpha), glm::mix(p2 - p1, p3 - p2, alpha), alpha);
	}

	Rect LinearCurve::Bounds() const {
		Rect aabb{p0};
		aabb.Encapsulate(p1);
		return aabb;
	}
	Rect QuadraticCurve::Bounds() const {
		Rect aabb{p0};
		aabb.Encapsulate(p2);
		glm::vec2 const bottom = (p1 - p0) - (p2 - p1);
		if (bottom.x != 0.0f) {
			float const alpha = (p1.x - p0.x) / bottom.x;
			if (alpha > 0 && alpha < 1) aabb.Encapsulate(Position(alpha));
		}
		if (bottom.y != 0.0f) {
			float const alpha = (p1.y - p0.y) / bottom.y;
			if (alpha > 0 && alpha < 1) aabb.Encapsulate(Position(alpha));
		}
		return aabb;
	}
	Rect CubicCurve::Bounds() const {
		Rect aabb{p0};
		aabb.Encapsulate(p3);
		glm::vec2 const a0 = p1 - p0;
		glm::vec2 const a1 = 2.0f * (p2 - p1 - a0);
		glm::vec2 const a2 = p3- (3.0f * p2) + (3.0f * p1) - p0;
		{
			float alphas[2];
			size_t const solutions = SolveQuadratic(alphas, a2.x, a1.x, a0.x);
			for (size_t index = 0; index < solutions; ++index) {
				if (alphas[index] > 0 && alphas[index] < 1) aabb.Encapsulate(Position(alphas[index]));
			}
		}
		{
			float alphas[2];
			size_t const solutions = SolveQuadratic(alphas, a2.y, a1.y, a0.y);
			for (size_t index = 0; index < solutions; ++index) {
				if (alphas[index] > 0 && alphas[index] < 1) aabb.Encapsulate(Position(alphas[index]));
			}
		}
		return aabb;
	}

	template<typename T>
	uint8_t NonZeroSign(T alpha) {
		return 2 * (alpha > T{0}) - 1;
	}

	CurveMinimumSignedDistance LinearCurve::MinimumSignedDistance(glm::vec2 origin) const {
		SignedDistance minimumSignedDistance;
		glm::vec2 const aq = origin - p0;
		glm::vec2 const ab = p1 - p0;
		float nearestAlpha = glm::dot(aq, ab) / glm::dot(ab, ab);
		glm::vec2 const eq = (nearestAlpha > 0.5f ? p1 : p0) - origin;
		float const endpointDistance = glm::length(eq);
		if (nearestAlpha > 0.0f && nearestAlpha < 1.0f) {
			float const abLength = glm::length( ab );
			glm::vec2 const abPerpendicular{ ab.y/abLength, -ab.x/abLength };
			float const orthoDistance = glm::dot( abPerpendicular, aq );
			if (fabs(orthoDistance) < endpointDistance) minimumSignedDistance = SignedDistance{orthoDistance, 0};
		} else {
			minimumSignedDistance = SignedDistance{
				NonZeroSign(glm::cross(aq, ab)) * endpointDistance,
				fabs(glm::dot(glm::normalize(ab), glm::normalize(eq)))
			};
		}
		return CurveMinimumSignedDistance{minimumSignedDistance, nearestAlpha};
	}
	CurveMinimumSignedDistance QuadraticCurve::MinimumSignedDistance(glm::vec2 origin) const {
		glm::vec2 const qa = p0 - origin;
		glm::vec2 const ab = p1 - p0;
		glm::vec2 const br = p0 + p2 - p1 - p1;
		float const a = glm::dot(br, br);
		float const b = 3.0f * glm::dot(ab, br);
		float const c = 2.0f * glm::dot(ab, ab) + glm::dot(qa, br);
		float const d = glm::dot(qa, ab);
		float alphas[3];
		uint8_t const solutions = SolveCubic(alphas, a, b, c, d);

		float minimumDistance = NonZeroSign(glm::cross(ab, qa)) * glm::length(qa); // distance from a
		float nearestAlpha = -glm::dot(qa, ab) / glm::dot(ab, ab);
		{
			float const distance = NonZeroSign(glm::cross(p2 - p1, p2 - origin)) * glm::length(p2 - origin); // distance from b
			if (fabs(distance) < fabs(minimumDistance)) {
				minimumDistance = distance;
				nearestAlpha = glm::dot(origin - p1, p2 - p1) / glm::dot(p2 - p1, p2 - p1);
			}
		}
		for (size_t index = 0; index < solutions; ++index) {
			if (alphas[index] > 0.0f && alphas[index] < 1.0f) {
				glm::vec2 const endpoint = p0 + (2.0f * alphas[index] * ab) + (alphas[index] * alphas[index] * br);
				float const distance = NonZeroSign(glm::cross(p2 - p0, endpoint - origin)) * glm::length(endpoint - origin);
				if (fabs(distance) <= fabs(minimumDistance)) {
					minimumDistance = distance;
					nearestAlpha = alphas[index];
				}
			}
		}

		if (nearestAlpha >= 0.0f && nearestAlpha <= 1.0f) {
			return CurveMinimumSignedDistance{SignedDistance{minimumDistance, 0.0f}, nearestAlpha};
		} if (nearestAlpha < 0.5f) {
			return CurveMinimumSignedDistance{SignedDistance{minimumDistance, fabs(glm::dot(glm::normalize(ab), glm::normalize(qa)))}, nearestAlpha};
		} else {
			return CurveMinimumSignedDistance{SignedDistance{minimumDistance, fabs(glm::dot(glm::normalize(p2 - p1), glm::normalize(p2 - origin)))}, nearestAlpha};
		}
	}
	CurveMinimumSignedDistance CubicCurve::MinimumSignedDistance(glm::vec2 origin) const {
		glm::vec2 const qa = p0 - origin;
		glm::vec2 const ab = p1 - p0;
		glm::vec2 const br = p2 - p1 - ab;
		glm::vec2 const as = (p3 - p2) - (p2 - p1) - br;

		glm::vec2 epDirection = Direction(0.0f);
		float minimumDistance = NonZeroSign(glm::cross(epDirection, qa)) * glm::length(qa); // distance from a
		float nearestAlpha = -glm::dot(qa, epDirection) / glm::dot(epDirection, epDirection);
		{
			epDirection = Direction(1.0f);
			float const distance = NonZeroSign(glm::cross(epDirection, p3 - origin)) * glm::length(p3 - origin); // distance from b
			if (fabs(distance) < fabs(minimumDistance)) {
				minimumDistance = distance;
				nearestAlpha = glm::dot(origin + epDirection - p3, epDirection) / glm::dot(epDirection, epDirection);
			}
		}
		// Iterative minimum distance search
		static constexpr size_t CubicSeachSegments = 10;
		static constexpr size_t CubicSeachSteps = 10;
		for (size_t index = 0; index <= CubicSeachSegments; ++index) {
			float alpha = (float)index / (float)CubicSeachSegments;
			for (size_t step = 0;; ++step) {
				glm::vec2 const qpt = Position(alpha) - origin;
				float const distance = NonZeroSign(glm::cross(Direction(alpha), qpt)) * glm::length(qpt);
				if (fabs(distance) < fabs(minimumDistance)) {
					minimumDistance = distance;
					nearestAlpha = alpha;
				}

				if (step == CubicSeachSteps) break;

				// Improve alphas
				glm::vec2 const d1 = (3.0f * as * alpha * alpha) + (6.0f * br * alpha) + (3.0f * ab);
				glm::vec2 const d2 = (6.0f * as * alpha) + (6.0f * br);
				alpha -= glm::dot(qpt, d1) / (glm::dot(d1, d1) + glm::dot(qpt, d2));
				if (alpha < 0.0f || alpha > 1.0f) break;
			}
		}

		if (nearestAlpha >= 0.0f && nearestAlpha <= 1.0f)
			return CurveMinimumSignedDistance{SignedDistance{minimumDistance, 0.0f}, nearestAlpha};
		if (nearestAlpha < 0.5f)
			return CurveMinimumSignedDistance{SignedDistance{minimumDistance, fabs(glm::dot(glm::normalize(Direction(0.0f)), glm::normalize(qa)))}, nearestAlpha};
		else
			return CurveMinimumSignedDistance{SignedDistance{minimumDistance, fabs(glm::dot(glm::normalize(Direction(1.0f)), glm::normalize(p3 - origin)))}, nearestAlpha};
	}

	void LinearCurve::SetStartPosition(glm::vec2 newStartPosition) {
		p0 = newStartPosition;
	}
	void QuadraticCurve::SetStartPosition(glm::vec2 newStartPosition) {
		glm::vec2 const originalP1 = p1;
		glm::vec2 const originalP01 = p0 - p1;
		glm::vec2 const originalP21 = p2 - p1;
		p1 += glm::cross(originalP01, newStartPosition - p0) / glm::cross(originalP01, originalP21) * originalP21;
		p0 = newStartPosition;
		if (glm::dot(originalP01, p0 - p1) < 0.0f) p1 = originalP1;
	}
	void CubicCurve::SetStartPosition(glm::vec2 newStartPosition) {
		p1 += (newStartPosition - p0);
		p0 = newStartPosition;
	}

	void LinearCurve::SetEndPosition(glm::vec2 newEndPosition) {
		p1 = newEndPosition;
	}
	void QuadraticCurve::SetEndPosition(glm::vec2 newEndPosition) {
		glm::vec2 const originalP1 = p1;
		glm::vec2 const originalP01 = p0 - p1;
		glm::vec2 const originalP21 = p2 - p1;
		p1 += glm::cross(originalP21, newEndPosition - p2) / glm::cross(originalP21, originalP01) * originalP01;
		p2 = newEndPosition;
		if( glm::dot(originalP21, p2 - p1) < 0.0f) p1 = originalP1;
	}
	void CubicCurve::SetEndPosition(glm::vec2 newEndPosition) {
		p2 += (newEndPosition - p3);
		p3 = newEndPosition;
	}
}
