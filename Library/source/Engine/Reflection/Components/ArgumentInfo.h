#pragma once
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Engine/StandardTypes.h"

namespace Reflection {
	struct TypeInfo;

	enum class EArgumentFlags : uint8_t {
		Optional,
		Const,
		Reference,
	};
	using FArgumentFlags = TFlags<EArgumentFlags>;

	//Reflection info for an argument to a function
	struct ArgumentInfo {
		std::string name;
		Hash32 id;
		TypeInfo const* type = nullptr;
		FArgumentFlags flags = FArgumentFlags::None();

		std::string description;
	};
}
