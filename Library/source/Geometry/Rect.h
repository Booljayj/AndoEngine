#pragma once
#include <limits>
#include <glm/vec2.hpp>

namespace Geometry {
	/** Represents a 2D axially-aligned bounding box */
	struct Rect {
		glm::vec2 Min;// = { std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
		glm::vec2 Max;// = { -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };

		Rect();
		Rect( const glm::vec2& Position );

		bool IsValid() const;

		bool Contains( const glm::vec2& Position ) const;
		bool Intersects( const Rect& Bounds ) const;
		bool Overlaps( const Rect& Bounds ) const;

		void Encapsulate( const glm::vec2& Position );
		void Encapsulate( const Rect& Bounds );
	};
}
