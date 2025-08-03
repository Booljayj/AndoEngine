#pragma once
#include "Engine/Core.h"

//String hash types. These store hashes in specialized types which encapsulate constructing, combining, and
//storing hashes of various lengths.

//Stable in this context means that the hash will not change between program executions or between platforms,
//which makes these hashes suitable as long-term identifiers.

//The input can either be a utf8 string or an ascii string, which will give different results. Providing proper
//input is left to the caller, so ensure that if you are matching a user input string hash to a hardcoded one,
//the string encoding is the same.

//Because these hashes are stable, the implementation CANNOT change after it has been used for any kind of
//long-term data storage. Bugs or changes will likely require a full version change and some data migration
//to update any hashes that are being used.

//Hashes can be default-constructed, but the default internal values may match a hash created from a string.
//Do not assume that a hash was default-constructed because it has any particular internal value.
//These also cannot be exposed for reflection, because they are used within the reflection system, but they
//may be serialized directly if needed.

/** A 32-bit stable string hash */
struct Hash32 {
	constexpr Hash32() = default;
	constexpr Hash32(std::span<char const> string, uint32_t seed = 0);
	explicit constexpr Hash32(uint32_t value) : hash(value) {}
	constexpr Hash32(const Hash32&) = default;

	constexpr bool operator==(const Hash32& other) const = default;
	constexpr bool operator!=(const Hash32& other) const = default;

	constexpr Hash32& operator+=(Hash32 second);
	friend constexpr Hash32 operator+(Hash32 first, Hash32 second) { return first.operator+=(second);  }

	constexpr uint32_t ToValue() const { return hash; }

private:
	static constexpr uint32_t C1 = 0xcc9e2d51;
	static constexpr uint32_t C2 = 0x1b873593;
	static constexpr size_t BlockSize = sizeof(uint32_t);

	uint32_t hash = 0;
};

/** A 64-bit stable string hash */
struct Hash64 {
	constexpr Hash64() = default;
	constexpr Hash64(std::span<char const> string, uint64_t seed = 0);
	explicit constexpr Hash64(uint64_t value) : hash(value) {}
	constexpr Hash64(const Hash64& other) = default;

	constexpr bool operator==(const Hash64& other) const = default;
	constexpr bool operator!=(const Hash64& other) const = default;

	constexpr Hash64& operator+=(Hash64 second);
	friend constexpr Hash64 operator+(Hash64 first, Hash64 second) { return first.operator+=(second); }

	constexpr uint64_t ToValue() const { return hash; }

private:
	static constexpr uint64_t C1 = 0x87c37b91114253d5;
	static constexpr uint64_t C2 = 0x4cf5ad432745937f;
	static constexpr size_t BlockSize = sizeof(uint64_t);

	uint64_t hash = 0;
};

/** A 128-bit stable string hash */
struct Hash128 {
	constexpr Hash128() = default;
	constexpr Hash128(std::span<char const> string, uint128_t seed = { 0, 0 });
	explicit constexpr Hash128(uint128_t hash) : hash(hash) {}
	constexpr Hash128(const Hash128& other) = default;

	constexpr bool operator==(const Hash128& other) const = default;
	constexpr bool operator!=(const Hash128& other) const = default;

	constexpr Hash128& operator+=(Hash128 second);
	friend constexpr Hash128 operator+(Hash128 first, Hash128 second) { return first.operator+=(second); }

	constexpr uint128_t ToValue() const { return hash; }

private:
	static constexpr uint64_t C1 = 0x87c37b91114253d5;
	static constexpr uint64_t C2 = 0x4cf5ad432745937f;
	static constexpr size_t BlockSize = sizeof(uint64_t) * 2;

	uint128_t hash = { 0, 0 };
};

constexpr Hash32 operator ""_h32(char const* p, size_t s) { return Hash32{ std::string_view{ p, s }, 0 }; };
constexpr Hash64 operator ""_h64(char const* p, size_t s) { return Hash64{ std::string_view{ p, s }, 0 }; };
constexpr Hash128 operator ""_h128(char const* p, size_t s) { return Hash128{ std::string_view{ p, s }, { 0, 0 } }; };

//=============================================================================
// Standard library support

template<>
struct std::hash<Hash32> {
	constexpr size_t operator()(Hash32 hash) const { return hash.ToValue(); }
};
template<>
struct std::hash<Hash64> {
	constexpr size_t operator()(Hash64 hash) const { return hash.ToValue(); }
};
template<>
struct std::hash<Hash128> {
	constexpr size_t operator()(Hash128 const& hash) const { return hash.ToValue().low ^ hash.ToValue().high; }
};

//=============================================================================
// Serialization support

namespace Archive {
	template<>
	struct Serializer<Hash32> {
		static void Write(Output& archive, Hash32 const hash);
		static void Read(Input& archive, Hash32& hash);
	};
	template<>
	struct Serializer<Hash64> {
		static void Write(Output& archive, Hash64 const hash);
		static void Read(Input& archive, Hash64& hash);
	};
	template<>
	struct Serializer<Hash128> {
		static void Write(Output& archive, Hash128 const& hash);
		static void Read(Input& archive, Hash128& hash);
	};
}

namespace YAML {
	class Node;
	template<typename T> class convert;

	template<>
	class convert<Hash32> {
		static Node encode(Hash32 hash);
		static bool decode(Node const& node, Hash32& hash);
	};
	template<>
	struct convert<Hash64> {
		static Node encode(Hash64 hash);
		static bool decode(Node const& node, Hash64& hash);
	};
	template<>
	struct convert<Hash128> {
		static Node encode(Hash128 hash);
		static bool decode(Node const& node, Hash128& hash);
	};
}

//=============================================================================
// String formatting support

template<>
struct std::formatter<Hash32> : std::formatter<std::string_view> {
	std::format_context::iterator format(const Hash32& hash, format_context& ctx) const;
};
template<>
struct std::formatter<Hash64> : std::formatter<std::string_view> {
	std::format_context::iterator format(const Hash64& hash, format_context& ctx) const;
};
template<>
struct std::formatter<Hash128> : std::formatter<std::string_view> {
	std::format_context::iterator format(const Hash128& hash, format_context& ctx) const;
};

//=============================================================================
// Constexpr method implementations

constexpr Hash32::Hash32(std::span<char const> string, uint32_t seed) : hash(seed) {
	//Calculate a 32-bit Murmur3 hash for the string view
	size_t const size = string.size();

	if (size > 0) {
		size_t const num_blocks = size / BlockSize;
		size_t const num_block_bytes = num_blocks * BlockSize;
		
		auto const blocks = string.subspan(0, num_block_bytes);
		auto const tail = string.subspan(num_block_bytes);
		
		//Body
		for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
			const auto block = std::span<char const, BlockSize>{ blocks.subspan(block_index * BlockSize, BlockSize) };

			uint32_t value = 0;
			for (size_t byte_index = 0; byte_index < sizeof(uint32_t); ++byte_index) value |= static_cast<uint32_t>(block[byte_index]) << (byte_index * 8);
			if constexpr (std::endian::native == std::endian::big) value = std::byteswap(value);

			value *= C1; value = std::rotl(value, 15); value *= C2;
			hash ^= value; hash = std::rotl(hash, 13); hash = hash * 5 + 0xe6546b64;
		}

		//Tail
		{
			uint32_t value = 0;

			switch (size & 3) {
			case 3: value ^= static_cast<uint32_t>(tail[2]) << (8 * 2); [[fallthrough]];
			case 2: value ^= static_cast<uint32_t>(tail[1]) << (8 * 1); [[fallthrough]];
			case 1: value ^= static_cast<uint32_t>(tail[0]) << (8 * 0);
			};

			value *= C1; value = std::rotl(value, 15); value *= C2;
			hash ^= value;
		}

		//Finalization
		const auto Mix = [](uint32_t k) -> uint32_t {
			k ^= k >> 16;
			k *= 0x85ebca6b;
			k ^= k >> 13;
			k *= 0xc2b2ae35;
			k ^= k >> 16;
			return k;
		};

		hash = Mix(hash ^ size);
	}
}

constexpr Hash32& Hash32::operator+=(Hash32 second) {
	second.hash *= C1; second.hash = std::rotl(second.hash, 15); second.hash *= C2;
	hash ^= second.hash; hash = std::rotl(hash, 13); hash = hash * 5 + 0xe6546b64;
	return *this;
}

constexpr Hash64::Hash64(std::span<char const> string, uint64_t seed) : hash(seed) {
	//Calculate a 64-bit Murmur3 hash for a string view
	//Not standard, extrapolated from the 32-bit and 128-bit hash implementations
	size_t const size = string.size();
	if (size > 0) {
		size_t const num_blocks = size / BlockSize;
		size_t const num_block_bytes = num_blocks * BlockSize;
		
		auto const blocks = string.subspan(0, num_block_bytes);
		auto const tail = string.subspan(num_block_bytes);

		//Body
		for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
			const auto block = std::span<char const, BlockSize>{ blocks.subspan(block_index * BlockSize, BlockSize) };

			uint64_t value = 0;
			for (size_t byte_index = 0; byte_index < sizeof(uint64_t); ++byte_index) value |= static_cast<uint64_t>(block[byte_index]) << (byte_index * 8);
			if constexpr (std::endian::native == std::endian::big) value = std::byteswap(value);
			
			value *= C1; value = std::rotl(value, 31); value *= C2;
			hash ^= value; hash = std::rotl(hash, 27); hash = hash * 5 + 0x52dce729;
		}

		//Tail
		{
			uint64_t value = 0;

			switch (size & 7) {
			case 7: value ^= static_cast<uint64_t>(tail[6]) << (8 * 6); [[fallthrough]];
			case 6: value ^= static_cast<uint64_t>(tail[5]) << (8 * 5); [[fallthrough]];
			case 5: value ^= static_cast<uint64_t>(tail[4]) << (8 * 4); [[fallthrough]];
			case 4: value ^= static_cast<uint64_t>(tail[3]) << (8 * 3); [[fallthrough]];
			case 3: value ^= static_cast<uint64_t>(tail[2]) << (8 * 2); [[fallthrough]];
			case 2: value ^= static_cast<uint64_t>(tail[1]) << (8 * 1); [[fallthrough]];
			case 1: value ^= static_cast<uint64_t>(tail[0]) << (8 * 0);
			};

			value *= C1; value = std::rotl(value, 31); value *= C2;
			hash ^= value;
		}

		// Finalization
		const auto Mix = [](uint64_t k) -> uint64_t {
			k ^= k >> 33;
			k *= 0xff51afd7ed558ccd;
			k ^= k >> 33;
			k *= 0xc4ceb9fe1a85ec53;
			k ^= k >> 33;
			return k;
		};

		hash = Mix(hash ^ size);
	}
}

constexpr Hash64& Hash64::operator+=(Hash64 second) {
	second.hash *= C1; second.hash = std::rotl(second.hash, 31); second.hash *= C2;
	hash ^= second.hash; hash = std::rotl(hash, 27); hash = hash * 5 + 0x52dce729;
	return *this;
}

constexpr Hash128::Hash128(std::span<char const> string, uint128_t seed) : Hash128(seed) {
	// Calculate a 128-bit Murmur3 hash for a string view
	size_t const size = string.size();
	if (size > 0) {
		size_t const num_blocks = size / BlockSize;
		size_t const num_block_bytes = num_blocks * BlockSize;
		
		auto const blocks = string.subspan(0, num_block_bytes);
		auto const tail = string.subspan(num_block_bytes);

		// Body
		for (size_t block_index = 0; block_index < num_blocks; ++block_index) {
			const auto block = std::span<char const, BlockSize>{ blocks.subspan(block_index * BlockSize, BlockSize) };

			const auto Load = [](std::span<char const, sizeof(uint64_t)> span) -> uint64_t
			{
				uint64_t value = 0;
				for (size_t byte_index = 0; byte_index < sizeof(uint64_t); ++byte_index) value |= static_cast<uint64_t>(span[byte_index]) << (byte_index * 8);
				if constexpr (std::endian::native == std::endian::big) value = std::byteswap(value);
				return value;
			};

			uint64_t value1 = Load(block.subspan<0, sizeof(uint64_t)>());
			uint64_t value2 = Load(block.subspan<sizeof(uint64_t), sizeof(uint64_t)>());
			
			value1 *= C1; value1 = std::rotl(value1, 31); value1 *= C2;
			value2 *= C2; value2 = std::rotl(value2, 33); value2 *= C1;
			hash.low ^= value1; hash.low = std::rotl(hash.low, 27); hash.low += hash.high; hash.low = hash.low * 5 + 0x52dce729;
			hash.high ^= value2; hash.high = std::rotl(hash.high, 31); hash.high += hash.low; hash.high = hash.high * 5 + 0x38495ab5;
		}

		// Tail
		{
			uint64_t value1 = 0;
			uint64_t value2 = 0;

			switch (size & 15) {
			case 15: value2 ^= static_cast<uint64_t>(tail[14]) << (8 * 6); [[fallthrough]];
			case 14: value2 ^= static_cast<uint64_t>(tail[13]) << (8 * 5); [[fallthrough]];
			case 13: value2 ^= static_cast<uint64_t>(tail[12]) << (8 * 4); [[fallthrough]];
			case 12: value2 ^= static_cast<uint64_t>(tail[11]) << (8 * 3); [[fallthrough]];
			case 11: value2 ^= static_cast<uint64_t>(tail[10]) << (8 * 2); [[fallthrough]];
			case 10: value2 ^= static_cast<uint64_t>(tail[9])  << (8 * 1); [[fallthrough]];
			case 9:  value2 ^= static_cast<uint64_t>(tail[8])  << (8 * 0); [[fallthrough]];

			case 8: value1 ^= static_cast<uint64_t>(tail[7]) << (8 * 7); [[fallthrough]];
			case 7: value1 ^= static_cast<uint64_t>(tail[6]) << (8 * 6); [[fallthrough]];
			case 6: value1 ^= static_cast<uint64_t>(tail[5]) << (8 * 5); [[fallthrough]];
			case 5: value1 ^= static_cast<uint64_t>(tail[4]) << (8 * 4); [[fallthrough]];
			case 4: value1 ^= static_cast<uint64_t>(tail[3]) << (8 * 3); [[fallthrough]];
			case 3: value1 ^= static_cast<uint64_t>(tail[2]) << (8 * 2); [[fallthrough]];
			case 2: value1 ^= static_cast<uint64_t>(tail[1]) << (8 * 1); [[fallthrough]];
			case 1: value1 ^= static_cast<uint64_t>(tail[0]) << (8 * 0);
			};

			value2 *= C2; value2 = std::rotl(value2, 33); value2 *= C1;
			hash.high ^= value2;

			value1 *= C1; value1 = std::rotl(value1, 31); value1 *= C2;
			hash.low ^= value1;
		}

		// Finalization
		hash.low ^= string.size();
		hash.high ^= string.size();

		hash.low += hash.high;
		hash.high += hash.low;

		const auto Mix = [](uint64_t k) -> uint64_t {
			k ^= k >> 33;
			k *= 0xff51afd7ed558ccdULL;
			k ^= k >> 33;
			k *= 0xc4ceb9fe1a85ec53ULL;
			k ^= k >> 33;
			return k;
		};

		hash.low = Mix(hash.low);
		hash.high = Mix(hash.high);

		hash.low += hash.high;
		hash.high += hash.low;
	}
}

constexpr Hash128& Hash128::operator+=(Hash128 second) {
	second.hash.low *= C1; second.hash.low = std::rotl(second.hash.low, 31); second.hash.low *= C2;
	second.hash.high *= C2; second.hash.high = std::rotl(second.hash.high, 33); second.hash.high *= C1;
	hash.low ^= second.hash.low; hash.low = std::rotl(hash.low, 27); hash.low += hash.high; hash.low = hash.low * 5 + 0x52dce729;
	hash.high ^= second.hash.high; hash.high = std::rotl(hash.high, 31); hash.high += hash.low; hash.high = hash.high * 5 + 0x38495ab5;
	return *this;
}
