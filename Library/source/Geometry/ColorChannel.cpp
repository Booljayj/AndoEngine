#include "Geometry/ColorChannel.h"

namespace Geometry {
	const FColorChannel FColorChannel::Yellow = FColorChannel::Make(EColorChannel::Red, EColorChannel::Green);
	const FColorChannel FColorChannel::Magenta = FColorChannel::Make(EColorChannel::Red, EColorChannel::Blue);
	const FColorChannel FColorChannel::Cyan = FColorChannel::Make(EColorChannel::Green, EColorChannel::Green);
	const FColorChannel FColorChannel::White = FColorChannel::Make(EColorChannel::Red, EColorChannel::Green, EColorChannel::Blue);
}
