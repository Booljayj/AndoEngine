#include "HAL/WindowingSystem.h"
#include "Engine/Logging.h"

namespace HAL {
	bool WindowingSystem::Startup() {
#if SDL_ENABLED
		primaryWindow = CreateWindow();
		if (!primaryWindow) {
			LOGF(SDL, Error, "Failed to create SDL window: %i", SDL_GetError());
			return false;
		}
		return true;
#else
		return false;
#endif
	}

	bool WindowingSystem::Shutdown() {
		for (Window const& window : windows) {
#if SDL_ENABLED
			SDL_DestroyWindow(window.handle);
#endif
		}

		windows.clear();
		primaryWindow = Window{};
		return true;
	}

	Window WindowingSystem::FindWindow(uint32_t id) const {
		const auto iter = std::find(windows.begin(), windows.end(), id);
		if (iter != windows.end()) return *iter;
		else return Window{};
	}

	Window WindowingSystem::CreateWindow() {
		Window::HandleType handle = SDL_CreateWindow("AndoSystem", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
		if (!handle) return Window{};

		Window window;
		window.id = SDL_GetWindowID(handle);
		window.handle = handle;

		windows.push_back(window);
		return window;
	}

	void WindowingSystem::DestroyWindow(uint32_t id) {
		//The primary window cannot be destroyed with this method, only during shutdown
		if (primaryWindow != id) {
			const auto iter = std::find(windows.begin(), windows.end(), id);
			if (iter != windows.end()) {
				SDL_DestroyWindow(iter->handle);
				windows.erase(iter);
			} else {
				LOGF(SDL, Warning, "Unable to destroy window, no window found with id %i", id);
			}
		}
	}
}
