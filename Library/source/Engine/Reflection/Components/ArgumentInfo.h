#pragma once
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Engine/StandardTypes.h"

namespace Reflection {
	struct TypeInfo;

	enum class EArgumentFlags : uint8_t {
		None = 0,
		Optional = 1 << 0,
		Const = 1 << 1,
		Reference = 1 << 2,
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
