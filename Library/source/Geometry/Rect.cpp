#include <algorithm>
#include "Geometry/Rect.h"

namespace Geometry {
	bool Between( float const& A, float const& Min, float const& Max ) {
		return A>=Min && A<=Max;
	}

	Rect::Rect()
	: Min( std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() )
	, Max( -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() )
	{}

	Rect::Rect( glm::vec2 const& Position )
	: Min( Position ), Max( Position )
	{}

	bool Rect::IsValid() const {
		return Min.x<=Max.x && Min.y<=Max.y;
	}

	bool Rect::Contains( const glm::vec2& Position ) const {
		return Between( Position.x, Min.x, Max.x ) && Between( Position.y, Min.y, Max.y );
	}
	bool Rect::Intersects( const Rect& Bounds ) const {
		bool const IntersectsX = Between( Bounds.Min.x, Min.x, Max.x ) || Between( Bounds.Max.x, Min.x, Max.x );
		bool const IntersectsY = Between( Bounds.Min.y, Min.y, Max.y ) || Between( Bounds.Max.y, Min.y, Max.y );
		return IntersectsX && IntersectsY;
	}
	bool Rect::Overlaps( const Rect& Bounds ) const {
		bool const IntersectsX = Between( Bounds.Min.x, Min.x, Max.x ) && Between( Bounds.Max.x, Min.x, Max.x );
		bool const IntersectsY = Between( Bounds.Min.y, Min.y, Max.y ) && Between( Bounds.Max.y, Min.y, Max.y );
		return IntersectsX && IntersectsY;
	}

	void Rect::Encapsulate( const glm::vec2& Position ) {
		Min.x = std::min( Min.x, Position.x );
		Min.y = std::min( Min.y, Position.y );
		Max.x = std::max( Max.x, Position.x );
		Max.y = std::max( Max.y, Position.y );
	}
	void Rect::Encapsulate( const Rect& Bounds ) {
		Min.x = std::min( Min.x, Bounds.Min.x );
		Min.y = std::min( Min.y, Bounds.Min.y );
		Max.x = std::max( Max.x, Bounds.Max.x );
		Max.y = std::max( Max.y, Bounds.Max.y );
	}
}
