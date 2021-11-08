#pragma once

namespace HAL {
	/** System that manages the high-level initialization and shutdown of the HAL framework */
	struct FrameworkSystem {
	public:
		bool Startup();
		bool Shutdown();
	};
}
