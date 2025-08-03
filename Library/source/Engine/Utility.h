#pragma once
#include "Engine/Concepts.h"
#include "Engine/Core.h"
#include "Engine/Ranges.h"

namespace Utility {
	/** Load a numeric value from a byte array. The byte array is assumed to be in little-endian order. */
	template<Concepts::Arithmetic T>
	constexpr inline void LoadOrdered(std::span<std::byte const, sizeof(T)> source, T& value) {
#ifdef __cpp_lib_start_lifetime_as
#pragma error "std::start_lifetime_as is now available, this method should be modified to make use of it."
#endif

		if constexpr (std::endian::native == std::endian::little) {
			uint64_t integer_value = 0;
			for (uint8_t byte_index = 0; byte_index < sizeof(T); ++byte_index) {
				integer_value |= (std::to_integer<uint64_t>(source[byte_index]) << (byte_index * CHAR_BIT));
			}
			value = static_cast<T>(integer_value);

			//value = *std::start_lifetime_as<T>(source.data());
		} else if constexpr (std::endian::native == std::endian::big) {
			uint64_t integer_value = 0;
			for (uint8_t byte_index = 0; byte_index < sizeof(T); ++byte_index) {
				integer_value |= (std::to_integer<uint64_t>(source[sizeof(T) - byte_index - 1]) << (byte_index * CHAR_BIT));
			}
			value = static_cast<T>(integer_value);

			//value = std::byteswap(*std::start_lifetime_as<T>(source.data()));
		} else {
			static_assert(false, "Mixed-endian platforms not supported");
		}
	}

	/** Save a numeric value to a byte array. The byte array is assumed to be in little-endian order. */
	template<Concepts::Arithmetic T>
	constexpr inline void SaveOrdered(T value, std::span<std::byte, sizeof(T)> target) {
		std::span<std::byte const, sizeof(T)> value_span = std::as_bytes(std::span<T, 1>{ &value, 1 });

		if constexpr (std::endian::native == std::endian::little) {
			for (size_t index = 0; index < sizeof(T); ++index) target[index] = value_span[index];
		} else if constexpr (std::endian::native == std::endian::big) {
			constexpr size_t last_index = sizeof(T) - 1;
			for (size_t index = 0; index < sizeof(T); ++index) target[index] = value_span[last_index - index];
		} else {
			static_assert(false, "Mixed-endian platforms not supported");
		}
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
