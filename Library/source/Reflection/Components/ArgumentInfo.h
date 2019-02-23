#pragma once
#include <string>

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
		std::string Name;
		std::string Description;
		TypeInfo const* Type = nullptr;

		FArgumentFlags Flags = FArgumentFlags::None;
	};
}
