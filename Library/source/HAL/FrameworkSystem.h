#pragma once
#include "Engine/Context.h"

namespace HAL {
	/** System that manages the high-level initialization and shutdown of the HAL framework */
	struct FrameworkSystem {
	public:
		bool Startup(CTX_ARG);
		bool Shutdown(CTX_ARG);
	};
}
