#include "Geometry/ColorChannel.h"

namespace Geometry {
	const FColorChannel FColorChannel::Yellow = { EColorChannel::Red, EColorChannel::Green };
	const FColorChannel FColorChannel::Magenta = { EColorChannel::Red, EColorChannel::Blue };
	const FColorChannel FColorChannel::Cyan = { EColorChannel::Green, EColorChannel::Blue };
	const FColorChannel FColorChannel::White = { EColorChannel::Red, EColorChannel::Green, EColorChannel::Blue };
}
