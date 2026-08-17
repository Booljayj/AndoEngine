#pragma once
#include "Engine/Core.h"
#include "Engine/Events.h"
#include "Engine/GLM.h"
#include "HAL/Window.h"
#include "HAL/WindowID.h"

namespace HAL {
	struct WindowCreationParams {
		std::string_view title;
		glm::ivec2 position;
		glm::ivec2 size;
		uint32_t flags;

		WindowCreationParams();
	};

	struct WindowFramework {
	public:
		using WindowContainer = std::vector<std::unique_ptr<Window>>;

		/** Broadcast just before a window is destroyed */
		TEvent<WindowID> window_destroyed;

		WindowFramework();
		~WindowFramework();

		/** Get the primary window, which is the first window created on startup */
		inline Window& GetPrimaryWindow() const { return *windows[0].get(); }

		/** Create a new window, returning its id */
		Window* CreateWindow(WindowCreationParams const& params);
		/** Find a window using its id */
		Window* FindWindow(WindowID id) const;
		/** Destroy a window using its id. Cannot be used to destroy the primary window. Returns true if a window was destroyed. */
		bool DestroyWindow(WindowID id);

		inline WindowContainer::const_iterator begin() const { return windows.begin(); }
		inline WindowContainer::const_iterator end() const { return windows.end(); }

	protected:
		WindowContainer windows;
	};
}
