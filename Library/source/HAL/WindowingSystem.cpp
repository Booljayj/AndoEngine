#include "HAL/WindowingSystem.h"
#include "Engine/Logging.h"
#include "HAL/SDL2.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"

namespace HAL {
	Window::~Window() {
		destroyed(id);
		SDL_DestroyWindow(handle);
	}

	Window::Window(WindowCreationParams const& params) {
		handle = SDL_CreateWindow(params.title.data(), params.position.x, params.position.y, params.size.x, params.size.y, params.flags | SDL_WINDOW_VULKAN);
		if (handle) {
			id = SDL_GetWindowID(handle);
		} else {
			LOGF(SDL, Error, "Failed to create SDL window: %i", SDL_GetError());
		}
	}

	bool WindowingSystem::Startup() {
		//Create one window on startup, which will be considered the "primary" window
		return !!CreateWindow(WindowCreationParams{});
	}

	bool WindowingSystem::Shutdown() {
		windows.clear();
		return true;
	}

	Window* WindowingSystem::FindWindow(Window::IdType id) const {
		auto const iter = std::find_if(windows.begin(), windows.end(), [&](auto const& window) { return window->id == id; });
		if (iter != windows.end()) return iter->get();
		else return nullptr;
	}

	Window* WindowingSystem::CreateWindow(WindowCreationParams const& params) {
		std::unique_ptr<Window> window = std::unique_ptr<Window>(new Window(params));
		if (!window->IsValid()) return nullptr;
		else return windows.emplace_back(std::move(window)).get();
	}

	bool WindowingSystem::DestroyWindow(Window::IdType id) {
		auto const iter = std::find_if(windows.begin(), windows.end(), [&](auto const& window) { return window->id == id; });
		if (iter != windows.end() && iter != windows.begin()) {
			windows.erase(iter);
			return true;
		} else {
			LOGF(SDL, Warning, "Unable to destroy window with id %i", id);
			return false;
		}
	}
}
