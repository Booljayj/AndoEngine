#pragma once

#pragma region STL
#include <cassert>
//Integer types
#include <cstddef>
#include <cstdint>
//Mathematical
#include <algorithm>
#include <limits>
#include <numbers>
//Streams
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
//Misc
#include <chrono>
#include <expected>
#include <filesystem>
#include <format>
#include <iterator>
#include <source_location>
#include <span>
#pragma endregion

/** Utility to convert a symbol to a string */
#define STRINGIFY(x) #x
#define STRINGIFY_U16(x) u#x
/** Utility to convert an expanded macro to a string */
#define STRINGIFY_MACRO(x) STRINGIFY(x)
#define STRINGIFY_MACRO_U16(x) STRINGIFY_U16(x)

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

/** A 128-bit integer. Primarily used to store values or perform comparisons, may not implement mathematical operations. */
struct uint128_t
{
	uint64_t low;
	uint64_t high;

	constexpr inline bool operator==(const uint128_t&) const = default;
	constexpr inline bool operator!=(const uint128_t&) const = default;
};

/** Combine two hash values to create a unique third hash value. Not commutative. */
constexpr inline uint64_t hash_combine(uint64_t a, uint64_t b) { return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2)); }

/** Extract the raw bytes from a stream. Does not limit the number of bytes that will be read, should not be used on infinite streams. */
template<std::output_iterator<std::byte> OutputIterator>
inline void ExtractBytes(std::istream& stream, OutputIterator output_iterator) {
	for (auto iter = std::istream_iterator<char>{ stream }; iter != std::default_sentinel; ++iter) {
		*output_iterator = std::byte{ static_cast<unsigned char>(*iter) };
		++output_iterator;
	}
}

template<typename T>
constexpr std::span<T const, 1> MakeSpan(const T& value) { return std::span<T const, 1>(&value, 1); }

//Declarations for archive serialization. Declared here to avoid some circular reference issues.
#pragma region Archive

namespace Archive {
	struct Output;
	struct Input;

	/** Write a span of bytes into the archive, in the exact order provided */
	void WriteBytes(Output& archive, std::span<std::byte const> source);

	/** Write a span of bytes into the archive, in the exact order provided */
	template<size_t N>
	inline void WriteBytes(Output& archive, std::span<std::byte const, N> source) {
		WriteBytes(archive, std::span<std::byte const>{ source });
	}

	/** Read a span of bytes from the archive. The resulting span has the same lifetime as the archive's source. */
	std::span<std::byte const> ReadBytes(Input& archive, size_t size);
	void Skip(Input& archive, size_t num);
	Input Subset(Input const& archive, size_t num);

	/** Read a span of bytes from the archive. The resulting span has the same lifetime as the archive's source. */
	template<size_t N>
	inline std::span<std::byte const, N> ReadBytes(Input& archive) {
		return std::span<std::byte const, N>{ ReadBytes(archive, N) };
	}

	/** Implemented by types that support serializing or deserializing from an archive. Support may not be bi-directional. */
	template<typename T>
	struct Serializer {
		//void Write(Output& archive, T const& value);
		//void Read(Input& archive, T& value);
	};

	/**
	 * Template method used to initialize an internal value before it is read.
	 * Typically the default constructor, but can be specialized for types that aren't normally default-constructible.
	 */
	template<typename T>
	inline T DefaultReadValue() { return T{}; }

	namespace Concepts {
		/** A type that can be written to an output archive */
		template<typename T>
		concept Writable = requires (T const& t, Output& archive) {
			{ Serializer<T>::Write(archive, t) };
		};
		
		/** A type that can be read from an input archive */
		template<typename T>
		concept Readable = requires (T& t, Input& archive) {
			{ Serializer<T>::Read(archive, t) };
		};

		/** A type that can be read from an input archive and written to an output archive */
		template<typename T>
		concept ReadWritable = Writable<T> and Readable<T>;
	}

	//=============================================================================
	// Basic serialization support

	/** Serializer for integral types */
	template<std::integral T>
	struct Serializer<T> {
		static constexpr size_t NumBytes = sizeof(T);

		static void Write(Output& archive, T const value) {
			if constexpr ((sizeof(T) == 1) || (std::endian::native == std::endian::little)) {
				WriteBytes(archive, std::as_bytes(MakeSpan(value)));
			} else {
				WriteBytes(archive, std::as_bytes(MakeSpan(&std::byteswap(value))));
			}
		}

		static void Read(Input& archive, T& value) {
#ifdef __cpp_lib_start_lifetime_as
#pragma error "std::start_lifetime_as is now available, this method should be modified to make use of it instead of memcpy"
#endif
			if constexpr (std::endian::native == std::endian::little) {
				const auto span = ReadBytes<NumBytes>(archive);
				std::memcpy(&value, span.data(), NumBytes);
			} else {
				const auto span = ReadBytes<NumBytes>(archive);
				std::memcpy(&value, span.data(), NumBytes);
				std::byteswap(value);
			}
		}
	};

	/** Serializer for floating-point types */
	template<std::floating_point T>
	struct Serializer<T> {
		using IntegerType = std::conditional_t<std::is_same_v<T, float>, uint32_t, uint64_t>;
		static constexpr size_t NumBytes = sizeof(T);

		static void Write(Output& archive, T const value) {
			IntegerType const integer_value = static_cast<IntegerType>(value);
			if constexpr (std::endian::native == std::endian::little) {
				WriteBytes(archive, std::as_bytes(MakeSpan(integer_value)));
			} else {
				WriteBytes(archive, std::as_bytes(MakeSpan(std::byteswap(integer_value))));
			}
		}

		static void Read(Input& archive, T& value) {
#ifdef __cpp_lib_start_lifetime_as
#pragma error "std::start_lifetime_as is now available, this method should be modified to make use of it instead of memcpy"
#endif
			IntegerType integer_value = 0;
			if constexpr (std::endian::native == std::endian::little) {
				const auto span = ReadBytes<NumBytes>(archive);
				std::memcpy(&integer_value, span.data(), NumBytes);
			} else {
				const auto span = ReadBytes<NumBytes>(archive);
				std::memcpy(&integer_value, span.data(), NumBytes);
				std::byteswap(integer_value);
			}
			value = static_cast<T>(integer_value);
		}
	};

	/** Serializer for boolean values */
	template<>
	struct Serializer<bool> {
		static inline void Write(Output& archive, bool const value) {
			//sizeof(bool) is implementation-defined, we'll ensure we only serialize a single byte regardless of the platform.
			uint8_t const integer_value = static_cast<uint8_t>(value);
			Serializer<uint8_t>::Write(archive, integer_value);
		}
		static inline void Read(Input& archive, bool& value) {
			//sizeof(bool) is implementation-defined, we'll ensure we only serialize a single byte regardless of the platform.
			uint8_t integer_value = 0;
			Serializer<uint8_t>::Read(archive, integer_value);
			value = static_cast<bool>(integer_value);
		}
	};

	/** Serializer for byte values */
	template<>
	struct Serializer<std::byte> {
		static inline void Write(Output& archive, std::byte const byte) {
			WriteBytes(archive, std::span<std::byte const, 1>{ &byte, 1 });
		}
		static inline void Read(Input& archive, std::byte& byte) {
			const auto span = ReadBytes<1>(archive);
			byte = span[0];
		}
	};
	template<>
	struct Serializer<std::byte const> {
		static inline void Write(Output& archive, std::byte const byte) {
			WriteBytes(archive, std::span<std::byte const, 1>{ &byte, 1 });
		}
	};

	/** Serializer for span values */
	template<Concepts::Writable T, size_t Extent>
	struct Serializer<std::span<T, Extent>> {
		static void Write(Output& archive, std::span<T, Extent> const& span) {
			if constexpr (Extent == std::dynamic_extent) {
				size_t const num = span.size();
				Serializer<size_t>::Write(archive, num);
			}

			for (const T& element : span) Serializer<T>::Write(archive, element);
		}
	};

	/** Serializer for byte span values */
	template<size_t Extent>
	struct Serializer<std::span<std::byte const, Extent>> {
		static void Write(Output& archive, std::span<std::byte const, Extent> const& span) {
			if constexpr (Extent == std::dynamic_extent) {
				size_t const num = span.size();
				Serializer<size_t>::Write(archive, num);
			}

			WriteBytes(archive, span);
		}
		static void Read(Input& archive, std::span<std::byte const, Extent>& span) {
			if constexpr (Extent == std::dynamic_extent) {
				size_t num = 0;
				Serializer<size_t>::Read(archive, num);
				span = ReadBytes(archive, num);
			} else {
				span = ReadBytes<Extent>(archive);
			}
		}
	};

	/** Serializer for std::pair values */
	template<typename FirstType, typename SecondType>
	struct Serializer<std::pair<FirstType, SecondType>> {
		void Write(Output& archive, std::pair<FirstType, SecondType> const& instance) {
			Serializer<FirstType>::Write(archive, instance.first);
			Serializer<SecondType>::Write(archive, instance.second);
		}
		void Read(Input& archive, std::pair<FirstType, SecondType>& instance) {
			Serializer<FirstType>::Read(archive, instance.first);
			Serializer<SecondType>::Read(archive, instance.second);
		}
	};
}

/** Implements operator<< as an alias for calling the Serializer::Write method for a type */
template<Archive::Concepts::Writable T>
Archive::Output& operator<<(Archive::Output& archive, T const& value) {
	Archive::Serializer<T>::Write(archive, value);
	return archive;
}

/** Implements operator>> as an alias for calling the Serializer::Read method for a type */
template<Archive::Concepts::Readable T>
Archive::Input& operator>>(Archive::Input& archive, T& value) {
	Archive::Serializer<T>::Read(archive, value);
	return archive;
}

#pragma endregion
