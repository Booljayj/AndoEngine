#include "HAL/Window.h"
#include "Engine/Format.h"
#include "HAL/WindowFramework.h"
#include "ThirdParty/SDL2.h"

namespace HAL {
	Window::~Window() {
		SDL_DestroyWindow(handle);
	}

	Window::Window(WindowCreationParams const& params) {
		handle = SDL_CreateWindow(params.title.data(), params.position.x, params.position.y, params.size.x, params.size.y, params.flags | SDL_WINDOW_VULKAN);
		if (!handle) throw FormatType<std::runtime_error>("Failed to create SDL window: {}", SDL_GetError());
		id = WindowID{ SDL_GetWindowID(handle) };
	}
}
