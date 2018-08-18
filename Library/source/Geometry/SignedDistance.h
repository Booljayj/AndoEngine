#pragma once
namespace Geometry
{
	struct SignedDistance {
		/** The distance from the reference location to the reference edge */
		float Distance;
		/** The dot product of the reference edge's surface tangent and the direction vector towards the reference location */
		float SurfaceTangentDot;
	};
}
