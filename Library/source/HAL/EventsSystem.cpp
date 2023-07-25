#include "HAL/EventsSystem.h"

namespace HAL {
	bool EventsSystem::Startup() {
		frameEvents.reserve(20);
		return true;
	}

	bool EventsSystem::Shutdown() { return true; }

	void EventsSystem::PollEvents(SystemEvents& system) {
		frameEvents.clear();

		SDL_Event currentEvent;

		while (SDL_PollEvent(&currentEvent)) {
			//ImGui_ImplSDL2_ProcessEvent(&currentEvent);
			frameEvents.push_back(currentEvent);

			system.quit |= (currentEvent.type == SDL_QUIT);
		}
	}
}
