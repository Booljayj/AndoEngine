#pragma once
#include "Engine/Archive.h"
#include "Engine/Math.h"
#include "Engine/StandardTypes.h"
#include "Engine/Utility.h"
#include "ThirdParty/yaml.h"

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

/** A 32-bit stable string hash */
struct Hash32 {
	constexpr Hash32() = default;
	constexpr Hash32(std::string_view string, uint32_t seed = 0);
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

	static constexpr uint32_t Mix(uint32_t k);
};

/** A 64-bit stable string hash */
struct Hash64 {
	constexpr Hash64() = default;
	constexpr Hash64(std::string_view string, uint64_t seed = 0);
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

	static constexpr uint64_t Mix(uint64_t k);
};

/** A 128-bit stable string hash */
struct Hash128 {
	constexpr Hash128() = default;
	constexpr Hash128(std::string_view string, uint64_t lowSeed = 0, uint64_t highSeed = 0);
	explicit constexpr Hash128(uint64_t low, uint64_t high) : low(low), high(high) {}
	constexpr Hash128(const Hash128& other) = default;

	constexpr bool operator==(const Hash128& other) const = default;
	constexpr bool operator!=(const Hash128& other) const = default;

	constexpr Hash128& operator+=(Hash128 second);
	friend constexpr Hash128 operator+(Hash128 first, Hash128 second) { return first.operator+=(second); }

	constexpr uint64_t ToLowValue() const { return low; }
	constexpr uint64_t ToHighValue() const { return high; }

private:
	static constexpr uint64_t C1 = 0x87c37b91114253d5;
	static constexpr uint64_t C2 = 0x4cf5ad432745937f;
	static constexpr size_t BlockSize = sizeof(uint64_t) * 2;

	uint64_t low = 0;
	uint64_t high = 0;

	static constexpr uint64_t Mix(uint64_t k);
};

constexpr Hash32 operator ""_h32(char const* p, size_t s) { return Hash32{ std::string_view{ p, s }, 0 }; };
constexpr Hash64 operator ""_h64(char const* p, size_t s) { return Hash64{ std::string_view{ p, s }, 0 }; };
constexpr Hash128 operator ""_h128(char const* p, size_t s) { return Hash128{ std::string_view{ p, s }, 0, 0 }; };

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
	constexpr size_t operator()(Hash128 const& hash) const { return hash.ToLowValue() ^ hash.ToHighValue(); }
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
	template<>
	struct convert<Hash32> {
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

#define L_CASE_TAIL_SHIFT(Value, Size, Offset) case Size: Value ^= static_cast<decltype(Value)>(tail[Size - 1]) << (8 * (Size - Offset - 1))

constexpr Hash32::Hash32(std::string_view string, uint32_t seed) : hash(seed) {
	//Calculate a 32-bit Murmur3 hash for the string view
	if (string.data() != nullptr && string.size() > 0) {
		size_t const count = string.size() / BlockSize;
		char const* const blocks = string.data();
		char const* const tail = string.data() + (count * BlockSize);

		//Body
		for (size_t index = 0; index < count; ++index) {
			char const* block = blocks + (index * BlockSize);
			uint32_t value = 0;
			Utility::LoadOrdered(std::span<char const, sizeof(uint32_t)>{ block, sizeof(uint32_t) }, value);

			value *= C1; value = Math::RotateLeft(value, 15); value *= C2;
			hash ^= value; hash = Math::RotateLeft(hash, 13); hash = hash * 5 + 0xe6546b64;
		}

		//Tail
		uint32_t value = 0;
		switch (string.size() & 3) {
			L_CASE_TAIL_SHIFT(value, 3, 0);
			L_CASE_TAIL_SHIFT(value, 2, 0);
			L_CASE_TAIL_SHIFT(value, 1, 0);
			value *= C1; value = Math::RotateLeft(value, 15); value *= C2;
			hash ^= value;
		};

		//Finalization
		hash ^= string.size();
		hash = Mix(hash);
	}
}

constexpr Hash32& Hash32::operator+=(Hash32 second) {
	second.hash *= C1; second.hash = Math::RotateLeft(second.hash, 15); second.hash *= C2;
	hash ^= second.hash; hash = Math::RotateLeft(hash, 13); hash = hash * 5 + 0xe6546b64;
	return *this;
}

constexpr uint32_t Hash32::Mix(uint32_t k) {
	k ^= k >> 16;
	k *= 0x85ebca6b;
	k ^= k >> 13;
	k *= 0xc2b2ae35;
	k ^= k >> 16;
	return k;
}

constexpr Hash64::Hash64(std::string_view string, uint64_t seed) : hash(seed) {
	//Calculate a 64-bit Murmur3 hash for a string view
	//Not standard, extrapolated from the 32-bit and 128-bit hash implementations
	if (string.data() != nullptr && string.size() > 0) {
		size_t const count = string.size() / BlockSize;
		char const* const blocks = string.data();
		char const* const tail = string.data() + (count * BlockSize);

		//Body
		for (size_t index = 0; index < count; index++) {
			char const* block = blocks + (index * BlockSize);
			uint64_t value = 0;
			Utility::LoadOrdered(std::span<char const, sizeof(uint64_t)>{ block, sizeof(uint64_t) }, value);
			
			value *= C1; value = Math::RotateLeft(value, 31); value *= C2;
			hash ^= value; hash = Math::RotateLeft(hash, 27); hash = hash * 5 + 0x52dce729;
		}

		//Tail
		uint64_t value = 0;
		switch (string.size() & 7) {
			L_CASE_TAIL_SHIFT(value, 7, 0);
			L_CASE_TAIL_SHIFT(value, 6, 0);
			L_CASE_TAIL_SHIFT(value, 5, 0);
			L_CASE_TAIL_SHIFT(value, 4, 0);
			L_CASE_TAIL_SHIFT(value, 3, 0);
			L_CASE_TAIL_SHIFT(value, 2, 0);
			L_CASE_TAIL_SHIFT(value, 1, 0);
			value *= C1; value = Math::RotateLeft(value, 31); value *= C2;
			hash ^= value;
		};

		// Finalization
		hash ^= string.size();
		hash = Mix(hash);
	}
}

constexpr Hash64& Hash64::operator+=(Hash64 second) {
	second.hash *= C1; second.hash = Math::RotateLeft(second.hash, 31); second.hash *= C2;
	hash ^= second.hash; hash = Math::RotateLeft(hash, 27); hash = hash * 5 + 0x52dce729;
	return *this;
}

constexpr uint64_t Hash64::Mix(uint64_t k) {
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccd;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53;
	k ^= k >> 33;
	return k;
}

constexpr Hash128::Hash128(std::string_view string, uint64_t lowSeed, uint64_t highSeed) : Hash128(lowSeed, highSeed) {
	// Calculate a 128-bit Murmur3 hash for a string view
	if (string.data() != nullptr && string.size() > 0) {
		uint64_t const count = string.size() / BlockSize;
		char const* const blocks = string.data();
		char const* const tail = string.data() + (count * BlockSize);

		// Body
		for (size_t index = 0; index < count; index++) {
			char const* block = blocks + (index * BlockSize);
			uint64_t value1 = 0;
			Utility::LoadOrdered(std::span<char const, sizeof(uint64_t)>{ block, sizeof(uint64_t) }, value1);
			uint64_t value2 = 0;
			Utility::LoadOrdered(std::span<char const, sizeof(uint64_t)>{ block + sizeof(uint64_t), sizeof(uint64_t) }, value1);

			value1 *= C1; value1 = Math::RotateLeft(value1, 31); value1 *= C2;
			value2 *= C2; value2 = Math::RotateLeft(value2, 33); value2 *= C1;
			low ^= value1; low = Math::RotateLeft(low, 27); low += high; low = low * 5 + 0x52dce729;
			high ^= value2; high = Math::RotateLeft(high, 31); high += low; high = high * 5 + 0x38495ab5;
		}

		// Tail
		uint64_t value1 = 0;
		uint64_t value2 = 0;
		switch (string.size() & 15) {
			L_CASE_TAIL_SHIFT(value2, 15, 8);
			L_CASE_TAIL_SHIFT(value2, 14, 8);
			L_CASE_TAIL_SHIFT(value2, 13, 8);
			L_CASE_TAIL_SHIFT(value2, 12, 8);
			L_CASE_TAIL_SHIFT(value2, 11, 8);
			L_CASE_TAIL_SHIFT(value2, 10, 8);
			L_CASE_TAIL_SHIFT(value2, 9, 8);
			value2 *= C2; value2 = Math::RotateLeft(value2, 33); value2 *= C1;
			high ^= value2;

			L_CASE_TAIL_SHIFT(value1, 8, 0);
			L_CASE_TAIL_SHIFT(value1, 7, 0);
			L_CASE_TAIL_SHIFT(value1, 6, 0);
			L_CASE_TAIL_SHIFT(value1, 5, 0);
			L_CASE_TAIL_SHIFT(value1, 4, 0);
			L_CASE_TAIL_SHIFT(value1, 3, 0);
			L_CASE_TAIL_SHIFT(value1, 2, 0);
			L_CASE_TAIL_SHIFT(value1, 1, 0);
			value1 *= C1; value1 = Math::RotateLeft(value1, 31); value1 *= C2;
			low ^= value1;
		};

		// Finalization
		low ^= string.size();
		high ^= string.size();

		low += high;
		high += low;

		low = Mix(low);
		high = Mix(high);

		low += high;
		high += low;
	}
}

constexpr Hash128& Hash128::operator+=(Hash128 second) {
	second.low *= C1; second.low = Math::RotateLeft(second.low, 31); second.low *= C2;
	second.high *= C2; second.high = Math::RotateLeft(second.high, 33); second.high *= C1;
	low ^= second.low; low = Math::RotateLeft(low, 27); low += high; low = low * 5 + 0x52dce729;
	high ^= second.high; high = Math::RotateLeft(high, 31); high += low; high = high * 5 + 0x38495ab5;
	return *this;
}

constexpr uint64_t Hash128::Mix(uint64_t k) {
	k ^= k >> 33;
	k *= 0xff51afd7ed558ccdULL;
	k ^= k >> 33;
	k *= 0xc4ceb9fe1a85ec53ULL;
	k ^= k >> 33;
	return k;
}

#undef L_CASE_TAIL_SHIFT
