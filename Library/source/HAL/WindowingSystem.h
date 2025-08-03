#pragma once
#include "Engine/Core.h"
#include "Engine/Events.h"
#include "Engine/GLM.h"
#include "HAL/SDL2.h"

namespace HAL {
	struct WindowCreationParams {
		std::string_view title;
		glm::ivec2 position = { SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED };
		glm::ivec2 size = { 1280, 800 };
		uint32_t flags = SDL_WINDOW_SHOWN;
	};

	struct Window {
		using HandleType = SDL_Window*;
		using IdType = uint32_t;

		static constexpr IdType Invalid = 0;

		/** The unique identifier that can be used to retrieve this window */
		IdType id = Invalid;
	
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		~Window();

		inline operator HandleType() const { return handle; }
		inline bool operator==(IdType otherID) const { return id == otherID; }
		inline bool operator==(HandleType otherHandle) const { return handle == otherHandle; }

	private:
		friend struct WindowingSystem;

		/** The low-level handle for this window */
		HandleType handle = nullptr;

		Window(WindowCreationParams const& params);
	};

	struct WindowingSystem {
	public:
		using WindowContainer = std::vector<std::unique_ptr<Window>>;

		/** Broadcast just before a window is destroyed */
		TEvent<Window::IdType> destroying;

		bool Startup();
		bool Shutdown();

		/** Get the primary window, which is the first window created on startup */
		inline Window& GetPrimaryWindow() const { return *windows[0].get(); }

		/** Create a new window, returning its id */
		Window* CreateWindow(WindowCreationParams const& params);
		/** Find a window using its id */
		Window* FindWindow(Window::IdType id) const;
		/** Destroy a window using its id. Cannot be used to destroy the primary window. Returns true if a window was destroyed. */
		bool DestroyWindow(Window::IdType id);

		inline WindowContainer::const_iterator begin() const { return windows.begin(); }
		inline WindowContainer::const_iterator end() const { return windows.end(); }

	protected:
		WindowContainer windows;
	};
}
