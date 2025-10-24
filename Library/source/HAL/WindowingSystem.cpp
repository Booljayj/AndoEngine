#include "HAL/WindowingSystem.h"
#include "Engine/Logging.h"
#include "Engine/Ranges.h"
#include "Engine/Format.h"
#include "HAL/SDL2.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"

namespace HAL {
	Window::~Window() {
		SDL_DestroyWindow(handle);
	}

	Window::Window(WindowCreationParams const& params) {
		handle = SDL_CreateWindow(params.title.data(), params.position.x, params.position.y, params.size.x, params.size.y, params.flags | SDL_WINDOW_VULKAN);
		if (!handle) throw FormatType<std::runtime_error>("Failed to create SDL window: {}", SDL_GetError());
		id = SDL_GetWindowID(handle);
	}

	bool WindowingSystem::Startup() {
		//Create one window on startup, which will be considered the "primary" window
		return !!CreateWindow(WindowCreationParams{});
	}

	bool WindowingSystem::Shutdown() {
		destroying.Broadcast(Window::Invalid);
		windows.clear();
		return true;
	}

	Window* WindowingSystem::FindWindow(Window::IdType id) const {
		auto const iter = ranges::find_if(windows, [&](auto const& window) { return window->id == id; });
		if (iter != windows.end()) return iter->get();
		else return nullptr;
	}

	Window* WindowingSystem::CreateWindow(WindowCreationParams const& params) {
		return windows.emplace_back(new Window(params)).get();
	}

	bool WindowingSystem::DestroyWindow(Window::IdType id) {
		auto const iter = ranges::find_if(windows, [&](auto const& window) { return window->id == id; });
		if (iter != windows.end() && iter != windows.begin()) {
			destroying.Broadcast((*iter)->id);
			windows.erase(iter);
			return true;
		} else {
			LOG(SDL, Warning, "Unable to destroy window with id {}", id);
			return false;
		}
	}
}
