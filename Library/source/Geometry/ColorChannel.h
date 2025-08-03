#pragma once
#include "Engine/Core.h"
#include "Engine/Flags.h"

namespace Geometry {
	/* Flags which specify color channels and various combinations of them */
	enum class EColorChannel : uint8_t {
		Red,
		Green,
		Blue,
	};

	struct FColorChannel : public TFlags<EColorChannel> {
		TFLAGS_METHODS(FColorChannel)
		static const FColorChannel Yellow;
		static const FColorChannel Magenta;
		static const FColorChannel Cyan;
		static const FColorChannel White;
	};
}
