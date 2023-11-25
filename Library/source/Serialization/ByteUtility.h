#pragma once
#include "Engine/StandardTypes.h"

namespace Serialization {
	/** True if the current platform is little-endian */
	consteval inline bool IsPlatformLittleEndian() {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return true;
#else
		return false;
#endif
	}

	/** Functions which implement serialization routines that read and write in little-endian format */
	template<typename T>
		requires std::is_integral_v<T> || std::is_floating_point_v<T>
	void WriteLE(T const* data, std::ostream& stream) {
		constexpr size_t Size = sizeof(T);

		char const* bytes = reinterpret_cast<char const*>(data);
		if constexpr (IsPlatformLittleEndian()) {
			stream.write(bytes, Size);
		} else {
			char reversed[Size];
			for (size_t index = 0; index < Size; ++index) reversed[Size - index - 1] = bytes[index];
			stream.write(reversed, Size);
		}
	}

	template<typename T>
		requires std::is_integral_v<T> || std::is_floating_point_v<T>
	void ReadLE(T* data, std::istream& stream) {
		constexpr size_t Size = sizeof(T);

		char* bytes = reinterpret_cast<char*>(data);
		if constexpr (IsPlatformLittleEndian()) {
			stream.read(bytes, Size);
		} else {
			char reversed[Size];
			stream.read(reversed, Size);
			for (size_t index = 0; index < Size; ++index) bytes[index] = reversed[Size - index - 1];
		}
	}
}
