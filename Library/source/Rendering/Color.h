#pragma once
#include "Geometry/GLM.h"
#include "Reflection/TypeResolver.h"

namespace Rendering {
	using Color = glm::vec<4, uint8_t>;
}

namespace Reflection {
	namespace Internal {
		DECLARE_RESOLVER(Rendering::Color);
	}
}
