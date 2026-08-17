#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "ThirdParty/SDL2.h"

namespace HAL {
	/** A union that contains a single event that has been processed. */
	using EventUnion = SDL_Event;

	/** System-wide events received when polling events each frame */
	struct SystemEvents {
		bool quit = false;
	};

	/** Manages input events received from the system each frame */
	struct EventFramework {
		EventFramework();

		void PollEvents(SystemEvents& system);

	protected:
		std::vector<EventUnion> frame_events;
	};
}
