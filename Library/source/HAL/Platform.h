#pragma once

/** The various platforms that we can compile for */
#define PLATFORM_WINDOWS 0
#define PLATFORM_OSX 1
#define PLATFORM_LINUX 2

namespace HAL {
	enum class EPlatform {
		Windows = PLATFORM_WINDOWS,
		OSX = PLATFORM_OSX,
		Linux = PLATFORM_LINUX,
	};
}

#ifndef PLATFORM
//Default the current platform to Windows unless specified during compilation
#define PLATFORM PLATFORM_WINDOWS
#endif

/** The enum value of the current platform */
#define PLATFORM_ENUM static_cast<HAL::EPlatform>(PLATFORM)

/** Whether the platform is a desktop platform */
#define PLATFORM_IS_DESKTOP (PLATFORM == PLATFORM_WINDOWS || PLATFORM == PLATFORM_OSX || PLATFORM == PLATFORM_LINUX)
/** Whether the platform is a console platform */
#define PLATFORM_IS_CONSOLE !PLATFORM_IS_DESKTOP
