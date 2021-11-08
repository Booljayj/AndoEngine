#include "HAL/EventsSystem.h"

namespace HAL {
	bool EventsSystem::Startup() {
		frameEvents.reserve(20);
		return true;
	}

	bool EventsSystem::Shutdown() { return true; }

	void EventsSystem::PollEvents(bool& requestShutdown) {
		frameEvents.clear();

#if SDL_ENABLED
		SDL_Event currentEvent;

		while (SDL_PollEvent(&currentEvent)) {
			//ImGui_ImplSDL2_ProcessEvent(&currentEvent);
			frameEvents.push_back(currentEvent);

			requestShutdown |= (currentEvent.type == SDL_QUIT);
		}
#endif
	}
}
