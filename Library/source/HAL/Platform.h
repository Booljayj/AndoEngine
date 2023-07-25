#pragma once

namespace HAL {
	enum class EPlatform {
		Windows,
		OSX,
		Linux,
		MAX,
	};

	constexpr EPlatform GetCurrentPlatform() {
#ifdef PLATFORM
		return EPlatform::PLATFORM;
#else
		return EPlatform::Windows;
#endif
	}

	constexpr bool IsPlatformDesktop() {
		EPlatform const Platform = GetCurrentPlatform();
		return Platform == EPlatform::Windows || Platform == EPlatform::OSX || Platform == EPlatform::Linux;
	}

	constexpr bool IsPlatformConsole() {
		return !IsPlatformDesktop();
	}
}
