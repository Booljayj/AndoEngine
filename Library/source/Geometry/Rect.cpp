#include <algorithm>
#include "Geometry/Rect.h"

namespace Geometry {
	bool Between(float const& a, float const& min, float const& max) {
		return a>=min && a<=max;
	}

	bool Rect::Contains(const glm::vec2& position) const {
		return Between(position.x, min.x, max.x) && Between(position.y, min.y, max.y);
	}

	bool Rect::Intersects(const Rect& bounds) const {
		bool const intersectsX = Between(bounds.min.x, min.x, max.x) || Between(bounds.max.x, min.x, max.x);
		bool const intersectsY = Between(bounds.min.y, min.y, max.y) || Between(bounds.max.y, min.y, max.y);
		return intersectsX && intersectsY;
	}

	bool Rect::Overlaps(const Rect& bounds) const {
		bool const intersectsX = Between(bounds.min.x, min.x, max.x) && Between(bounds.max.x, min.x, max.x);
		bool const intersectsY = Between(bounds.min.y, min.y, max.y) && Between(bounds.max.y, min.y, max.y);
		return intersectsX && intersectsY;
	}

	void Rect::Encapsulate(const glm::vec2& position) {
		min.x = std::min(min.x, position.x);
		min.y = std::min(min.y, position.y);
		max.x = std::max(max.x, position.x);
		max.y = std::max(max.y, position.y);
	}

	void Rect::Encapsulate(const Rect& bounds) {
		min.x = std::min(min.x, bounds.min.x);
		min.y = std::min(min.y, bounds.min.y);
		max.x = std::max(max.x, bounds.max.x);
		max.y = std::max(max.y, bounds.max.y);
	}
}
