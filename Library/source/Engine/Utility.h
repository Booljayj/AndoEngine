#pragma once
#include "Engine/StandardTypes.h"

/** Utility to convert a symbol to a string */
#define STRINGIFY(x) #x
/** Utility to convert an expanded macro to a string */
#define STRINGIFY_MACRO(x) STRINGIFY(x)

/** Expands to a human-readable description of the compiler used when compiling */
#if defined(_MSC_VER)
#define COMPILER_VERSION "Microsoft C/C++ Compiler v" STRINGIFY_MACRO(_MSC_VER)
#elif defined(__clang__)
#define COMPILER_VERSION __VERSION__
#elif defined(__GNUC__)
#define COMPILER_VERSION __VERSION__
#else
#define COMPILER_VERSION "Unknown Compiler"
#endif

namespace Utility {
	/** Calculates floor(log2(value)) */
	constexpr size_t FloorLog2(size_t value) {
		return value == 1 ? 0 : 1 + FloorLog2(value >> 1);
	}

	/** Calculates the minimum number of bits required to store a maximum value. Also equal to ceil(log2(value)). */
	constexpr size_t GetMinimumNumBits(size_t max) {
		return max == 1 ? 0 : FloorLog2(max - 1) + 1;
	}

	/** Load a value from a byte array. The byte array is assumed to be in little-endian order. */
	template<stdext::numeric T>
	constexpr inline void LoadOrdered(std::span<std::byte const, sizeof(T)> source, T& value) {
		for (size_t index = 0; index < sizeof(T); ++index) {
			T const byte = static_cast<T>(std::to_integer<uint8_t>(source[index]));
			value |= (byte << (CHAR_BIT * index));
		}
	}
	/** Save a value to a byte array. The byte array is assumed to be in little-endian order. */
	template<stdext::numeric T>
	constexpr inline void SaveOrdered(T value, std::span<std::byte, sizeof(T)> target) {
		for (size_t index = 0; index < sizeof(T); ++index) {
			uint8_t const byte = static_cast<uint8_t>(value >> (CHAR_BIT * index));
			target[index] = std::byte{ byte };
		}
	}

	/** Load a value from a character array. The character array is assumed to be in little-endian order. */
	template<stdext::numeric T>
	constexpr inline void LoadOrdered(std::span<char const, sizeof(T)> source, T& value) {
		for (size_t index = 0; index < sizeof(T); ++index) {
			value |= (static_cast<T>(source[index]) << (CHAR_BIT * index));
		}
	}

	/** Save a value to a character array. The character array is assumed to be in little-endian order. */
	template<stdext::numeric T>
	constexpr inline void SaveOrdered(T value, std::span<char, sizeof(T)> target) {
		for (size_t index = 0; index < sizeof(T); ++index) {
			target[index] = static_cast<uint8_t>(value >> (CHAR_BIT * index));
		}
	}

	/** Extract the raw bytes from a stream. Does not limit the number of bytes that will be read, should not be used on infinite streams. */
	inline void ExtractBytes(std::istream& stream, std::vector<std::byte>& output) {
		std::for_each(
			std::istream_iterator<char>{ stream }, std::istream_iterator<char>{},
			[&output](char c) { output.push_back(std::byte{ static_cast<unsigned char>(c) }); }
		);
	}

	/** Extract the raw bytes from a stream. Does not limit the number of bytes that will be read, should not be used on infinite streams. */
	inline std::vector<std::byte> ExtractBytes(std::istream& stream) {
		std::vector<std::byte> result;
		ExtractBytes(stream, result);
		return result;
	}
}

/** A size value in bytes. Used as a wrapper to distinguish a byte size from a raw number. */
struct ByteSize {
	uint64_t size = 0;
	ByteSize(uint64_t size) : size(size) {}
};

template<>
struct std::formatter<ByteSize> : std::formatter<std::string_view> {
	/** Format a number as a fixed-point value, where the least significant digit is the tenths precision */
	auto format_fixed_point(format_context& ctx, char prefix, uint64_t value) const {
		return std::format_to(ctx.out(), "{}.{}{}B", value / 10u, value % 10u, prefix);
	}

	auto format(const ByteSize& bytes, format_context& ctx) const {
		uint64_t const size = bytes.size;
		if (size < 1000L) {
			return std::format_to(ctx.out(), "{}B", size);
		} else if (size < 999'950u) {
			return format_fixed_point(ctx, 'k', size / 100u);
		} else if (size < 999'950'000u) {
			return format_fixed_point(ctx, 'M', size / 100'000u);
		} else if (size < 999'950'000'000u) {
			return format_fixed_point(ctx, 'G', size / 100'000'000u);
		} else if (size < 999'950'000'000'000u) {
			return format_fixed_point(ctx, 'T', size / 100'000'000'000u);
		} else if (size < 999'950'000'000'000'000u) {
			return format_fixed_point(ctx, 'P', size / 100'000'000'000'000u);
		} else {
			return format_fixed_point(ctx, 'E', size / 100'000'000'000'000'000u);
		}
	}
};
