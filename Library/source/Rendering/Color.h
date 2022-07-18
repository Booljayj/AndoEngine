#pragma once
#include "Geometry/GLM.h"
#include "Reflection/TypeResolver.h"

namespace Rendering {
	using Color = glm::vec<4, uint8_t>;
}

namespace Reflection {
	DECLARE_RESOLVER(Rendering::Color);
}
