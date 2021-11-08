#pragma once
#include "Engine/STL.h"
#include "HAL/SDL2.h"

namespace HAL {
#if SDL_ENABLED
	using EventUnion = SDL_Event;
#else
	using EventUnion = uint8_t;
#endif

	/** Manages input events received from the system each frame */
	struct EventsSystem {
	protected:
		std::vector<EventUnion> frameEvents;

	public:
		bool Startup();
		bool Shutdown();

		void PollEvents(bool& requestShutdown);
	};
}
