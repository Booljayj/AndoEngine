#include "HAL/WindowFramework.h"
#include "Engine/Logging.h"
#include "Engine/Ranges.h"
#include "Engine/Format.h"
#include "ThirdParty/SDL2.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl.h"

namespace HAL {
	WindowCreationParams::WindowCreationParams()
		: title("window"sv)
		, position({ SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED })
		, size({ 1280, 800 })
		, flags(SDL_WINDOW_SHOWN)
	{}

	WindowFramework::WindowFramework() {
		LOG(Application, Info, "Starting windowing framework");
		CreateWindow(WindowCreationParams{});
	}

	WindowFramework::~WindowFramework() {
		LOG(Application, Info, "Stopping windowing framework");
		window_destroyed.Broadcast(WindowID{});
		windows.clear();
	}

	Window* WindowFramework::FindWindow(WindowID id) const {
		auto const iter = ranges::find_if(windows, [&](auto const& window) { return window->id == id; });
		if (iter != windows.end()) return iter->get();
		else return nullptr;
	}

	Window* WindowFramework::CreateWindow(WindowCreationParams const& params) {
		return windows.emplace_back(new Window(params)).get();
	}

	bool WindowFramework::DestroyWindow(WindowID id) {
		auto const iter = ranges::find_if(windows, [&](auto const& window) { return window->id == id; });
		if (iter != windows.end() && iter != windows.begin()) {
			window_destroyed.Broadcast((*iter)->id);
			windows.erase(iter);
			return true;
		} else {
			LOG(SDL, Warning, "Unable to destroy window with id {}", id);
			return false;
		}
	}
}
