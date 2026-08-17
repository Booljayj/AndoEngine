#pragma once
#include "Engine/Core.h"
#include "HAL/WindowID.h"

struct SDL_Window;

namespace HAL {
	struct WindowCreationParams;

	struct Window {
		using HandleType = SDL_Window*;
		
		/** The unique identifier that can be used to retrieve this window */
		WindowID id;

		Window(const Window&) = delete;
		Window(Window&&) = delete;
		~Window();

		inline operator HandleType() const { return handle; }
		inline bool operator==(WindowID otherID) const { return id == otherID; }
		inline bool operator==(HandleType otherHandle) const { return handle == otherHandle; }

	private:
		friend struct WindowFramework;

		/** The low-level handle for this window */
		HandleType handle = nullptr;

		Window(WindowCreationParams const& params);
	};
}
