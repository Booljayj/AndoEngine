#pragma once
#include <glm/vec2.hpp>
#include "Engine/STL.h"

namespace Geometry {
	/** Represents a 2D axially-aligned bounding box */
	struct Rect {
		glm::vec2 min = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
		glm::vec2 max = { -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };

		constexpr Rect() = default;
		constexpr Rect(glm::vec2 const& position)
		: min(position), max(position)
		{}

		inline constexpr bool IsValid() const { return min.x<=max.x && min.y<=max.y; }

		bool Contains(glm::vec2 const& position) const;
		bool Intersects(Rect const& bounds) const;
		bool Overlaps(Rect const& bounds) const;

		void Encapsulate(glm::vec2 const& position);
		void Encapsulate(Rect const& bounds);
	};
}
