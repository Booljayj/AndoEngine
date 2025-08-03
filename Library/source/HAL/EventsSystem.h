#pragma once
#include "Engine/Core.h"
#include "Engine/Vector.h"
#include "HAL/SDL2.h"

namespace HAL {
	using EventUnion = SDL_Event;

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
