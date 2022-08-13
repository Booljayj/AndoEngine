#pragma once
#include "Engine/Hash.h"
#include "Engine/StandardTypes.h"

namespace Reflection {
	struct TypeInfo;

	enum class FArgumentFlags : uint8_t {
		None = 0,
		Optional = 1 << 0,
		Const = 1 << 1,
		Reference = 1 << 2,
	};

	//Reflection info for an argument to a function
	struct ArgumentInfo {
		std::string name;
		Hash32 id;
		TypeInfo const* type = nullptr;
		FArgumentFlags flags = FArgumentFlags::None;

		std::string description;
	};
}
