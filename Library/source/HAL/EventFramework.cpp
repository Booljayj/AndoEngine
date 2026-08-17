#include "HAL/EventFramework.h"

namespace HAL {
	EventFramework::EventFramework() {
		frame_events.reserve(20);
	}

	void EventFramework::PollEvents(SystemEvents& system) {
		frame_events.clear();

		SDL_Event current_event;

		while (SDL_PollEvent(&current_event)) {
			//ImGui_ImplSDL2_ProcessEvent(&current_event);
			frame_events.push_back(current_event);

			system.quit |= (current_event.type == SDL_QUIT);
		}
	}
}
