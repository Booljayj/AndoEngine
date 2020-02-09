#pragma once
#include <cstdint>
#include "Engine/Flags.h"

namespace Geometry {
	/* Flags which specify color channels and various combinations of them */
	enum class EColorChannel : uint8_t {
		Red,
		Green,
		Blue,
	};

	struct FColorChannel : public TFlags<EColorChannel> {
		static const FColorChannel Yellow;
		static const FColorChannel Magenta;
		static const FColorChannel Cyan;
		static const FColorChannel White;
	};
	const FColorChannel FColorChannel::Yellow{{EColorChannel::Red, EColorChannel::Green}};
	const FColorChannel FColorChannel::Magenta{{EColorChannel::Red, EColorChannel::Blue}};
	const FColorChannel FColorChannel::Cyan{{EColorChannel::Green, EColorChannel::Green}};
	const FColorChannel FColorChannel::White{{EColorChannel::Red, EColorChannel::Green, EColorChannel::Blue}};
}
