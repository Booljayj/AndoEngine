#pragma once
#include "Engine/StandardTypes.h"
#include "HAL/SDL2.h"

namespace HAL {
#if SDL_ENABLED
	using EventUnion = SDL_Event;
#else
	using EventUnion = uint8_t;
#endif

	/** System-wide events received when polling events each frame */
	struct SystemEvents {
		bool quit = false;
	};

	/** Manages input events received from the system each frame */
	struct EventsSystem {
	protected:
		std::vector<EventUnion> frameEvents;

	public:
		bool Startup();
		bool Shutdown();

		void PollEvents(SystemEvents& system);
	};
}
