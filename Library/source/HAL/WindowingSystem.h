#pragma once
#include "Engine/Context.h"
#include "Engine/STL.h"
#include "HAL/SDL2.h"

namespace HAL {
	struct Window {
#if SDL_ENABLED
		using HandleType = SDL_Window*;
#else
		using HandleType = void*;
#endif

		uint32_t id = std::numeric_limits<uint32_t>::max();
		HandleType handle = nullptr;

		inline operator bool() const { return !!handle; }
		inline bool operator==(uint32_t otherID) const { return id == otherID; }
		inline bool operator==(HandleType otherHandle) const { return handle == otherHandle; }
	};

	struct WindowingSystem {
	public:
		bool Startup(CTX_ARG);
		bool Shutdown(CTX_ARG);

		inline Window GetPrimaryWindow() const { return primaryWindow; }

		/** Find a window using its id */
		Window FindWindow(uint32_t id) const;
		//@todo Add parameters to this method to control how the window should be created
		/** Create a new window, returning its id */
		Window CreateWindow(CTX_ARG);
		/** Destroy a window using its id */
		void DestroyWindow(CTX_ARG, uint32_t id);

	protected:
		Window primaryWindow;
		std::vector<Window> windows;
	};
}
