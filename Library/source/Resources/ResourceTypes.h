#pragma once
#include "Engine/Flags.h"
#include "Engine/Logging.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Engine/StringID.h"

namespace Resources {
	DECLARE_LOG_CATEGORY(Resources);
	
	/** Flag values which can apply to a particular resource */
	enum class EResourceFlags : uint32_t {
		/** Indicates this resource has been fully initialized */
		Initialized,
	};
	using FResourceFlags = TFlags<EResourceFlags>;

	struct Identifier {
		/** The name of the package in which the resource can be found */
		StringID package;
		/** The name of the resource that this identifier points to */
		StringID resource;
	};
}
