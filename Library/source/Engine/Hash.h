#pragma once
#include "Engine/Math.h"
#include "Engine/STL.h"
#include "Engine/Utility.h"

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

#define L_CASE_TAIL_SHIFT(Value, Size, Offset) case Size: Value ^= static_cast<decltype(Value)>(tail[Size - 1]) << (8*(Size - Offset - 1))

/** A 32-bit string hash */
struct Hash32 {
public:
	uint32_t hash;

	constexpr Hash32() : hash(0) {}
	explicit constexpr Hash32(uint32_t inValue) : hash(inValue) {}
	constexpr Hash32(const Hash32& other) : hash(other.hash) {}
	/** Primary constructor, which creates the hash for some string */
	constexpr Hash32(std::string_view string, uint32_t seed = 0) : hash(seed) {
		//Calculate a 32-bit Murmur3 hash for the string view
		if (string.data() != nullptr && string.size() > 0) {
			size_t const count = string.size() / BlockSize;
			char const* const blocks = string.data();
			char const* const tail = string.data() + (count*BlockSize);

			//Body
			for (size_t index = 0; index < count; ++index) {
				char const* block = blocks + (index*BlockSize);
				uint32_t value = Utility::Load<uint32_t>(block);
				value *= C1; value = Math::RotateLeft( value, 15 ); value *= C2;
				hash ^= value; hash = Math::RotateLeft( hash, 13 ); hash = hash*5 + 0xe6546b64;
			}

			//Tail
			uint32_t value = 0;
			switch (string.size() & 3) {
				L_CASE_TAIL_SHIFT(value, 3, 0);
				L_CASE_TAIL_SHIFT(value, 2, 0);
				L_CASE_TAIL_SHIFT(value, 1, 0);
				value *= C1; value = Math::RotateLeft( value, 15 ); value *= C2;
				hash ^= value;
			};

			//Finalization
			hash ^= string.size();
			hash = Mix( hash );
		}
	}

	constexpr bool operator==(const Hash32& other) const { return hash == other.hash; }
	constexpr bool operator!=(const Hash32& other) const { return !operator==(other); }

	constexpr Hash32& operator+=(Hash32 second) {
		second.hash *= C1; second.hash = Math::RotateLeft(second.hash, 15); second.hash *= C2;
		hash ^= second.hash; hash = Math::RotateLeft(hash, 13); hash = hash*5 + 0xe6546b64;
		return *this;
	}
	friend constexpr Hash32 operator+(Hash32 first, Hash32 second) {
		return first.operator+=(second);
	}

private:
	static constexpr uint32_t C1 = 0xcc9e2d51UL;
	static constexpr uint32_t C2 = 0x1b873593UL;
	static constexpr size_t BlockSize = sizeof(uint32_t);

	static constexpr uint32_t Mix(uint32_t k) {
		k ^= k >> 16;
		k *= 0x85ebca6b;
		k ^= k >> 13;
		k *= 0xc2b2ae35;
		k ^= k >> 16;
		return k;
	}
};
constexpr Hash32 operator ""_h32(char const* p, size_t s) { return Hash32{std::string_view{p, s}, 0}; };

/** A 64-bit string hash */
struct Hash64 {
private:
	static constexpr uint64_t C1 = 0x87c37b91114253d5ULL;
	static constexpr uint64_t C2 = 0x4cf5ad432745937fULL;
	static constexpr size_t BlockSize = sizeof(uint64_t);

	static constexpr uint64_t Mix(uint64_t k) {
		k ^= k >> 33;
		k *= 0xff51afd7ed558ccdULL;
		k ^= k >> 33;
		k *= 0xc4ceb9fe1a85ec53ULL;
		k ^= k >> 33;
		return k;
	}

public:
	uint64_t hash;

	constexpr Hash64() : hash(0) {}
	explicit constexpr Hash64(uint64_t inValue) : hash(inValue) {}
	constexpr Hash64(const Hash64& other) : hash(other.hash) {}
	/** Primary constructor, which creates the hash for some string */
	constexpr Hash64(std::string_view string, uint64_t seed = 0) : hash(seed) {
		//Calculate a 64-bit Murmur3 hash for a string view
		//Not standard, extrapolated from the 32-bit and 128-bit hash implementations
		if (string.data() != nullptr && string.size() > 0) {
			size_t const count = string.size() / BlockSize;
			char const* const blocks = string.data();
			char const* const tail = string.data() + (count*BlockSize);

			//Body
			for (size_t index = 0; index < count; index++) {
				char const* block = blocks + (index*BlockSize);
				uint64_t value = Utility::Load<uint64_t>(block);
				value *= C1; value = Math::RotateLeft( value, 31 ); value *= C2;
				hash ^= value; hash = Math::RotateLeft( hash, 27 ); hash = hash*5 + 0x52dce729;
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

	constexpr bool operator==(const Hash64& other) const { return hash == other.hash; }
	constexpr bool operator!=(const Hash64& other) const { return !operator==(other); }

	constexpr Hash64& operator+=(Hash64 second) {
		second.hash *= C1; second.hash = Math::RotateLeft(second.hash, 31); second.hash *= C2;
		hash ^= second.hash; hash = Math::RotateLeft(hash, 27); hash = hash*5 + 0x52dce729;
		return *this;
	}
	friend constexpr Hash64 operator+(Hash64 first, Hash64 second) {
		return first.operator+=(second);
	}
};
constexpr Hash64 operator ""_h64(char const* p, size_t s) { return Hash64{std::string_view{p, s}, 0}; };

/** A 128-bit string hash */
struct Hash128 {
private:
	static constexpr uint64_t C1 = 0x87c37b91114253d5ULL;
	static constexpr uint64_t C2 = 0x4cf5ad432745937fULL;
	static constexpr size_t BlockSize = sizeof(uint64_t) * 2;

	static constexpr uint64_t Mix(uint64_t k) {
		k ^= k >> 33;
		k *= 0xff51afd7ed558ccdULL;
		k ^= k >> 33;
		k *= 0xc4ceb9fe1a85ec53ULL;
		k ^= k >> 33;
		return k;
	}

public:
	uint64_t low;
	uint64_t high;

	constexpr Hash128() : low(0), high(0) {}
	explicit constexpr Hash128(uint64_t inLow, uint64_t inHigh) : low(inLow), high(inHigh) {}
	constexpr Hash128(const Hash128& other) : low(other.low), high(other.high) {}
	/** Primary constructor, which creates the hash for some string */
	constexpr Hash128(std::string_view string, uint64_t lowSeed = 0, uint64_t highSeed = 0) : Hash128(lowSeed, highSeed) {
		// Calculate a 128-bit Murmur3 hash for a string view
		if (string.data() != nullptr && string.size() > 0) {
			uint64_t const count = string.size() / BlockSize;
			char const* const blocks = string.data();
			char const* const tail = string.data() + (count*BlockSize);

			// Body
			for(size_t index = 0; index < count; index++) {
				char const* block = blocks + (index*BlockSize);
				uint64_t value1 = Utility::Load<uint64_t>(block);
				uint64_t value2 = Utility::Load<uint64_t>(block, sizeof(uint64_t));

				value1 *= C1; value1 = Math::RotateLeft(value1, 31); value1 *= C2;
				value2 *= C2; value2 = Math::RotateLeft(value2, 33); value2 *= C1;
				low ^= value1; low = Math::RotateLeft(low, 27); low += high; low = low*5 + 0x52dce729;
				high ^= value2; high = Math::RotateLeft(high, 31); high += low; high = high*5 + 0x38495ab5;
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

	constexpr bool operator==(const Hash128& other) const { return low == other.low && high == other.high; }
	constexpr bool operator!=(const Hash128& other) const { return !operator==(other); }

	constexpr Hash128& operator+=(Hash128 second) {
		second.low *= C1; second.low = Math::RotateLeft(second.low, 31); second.low *= C2;
		second.high *= C2; second.high = Math::RotateLeft(second.high, 33); second.high *= C1;
		low ^= second.low; low = Math::RotateLeft(low, 27); low += high; low = low*5 + 0x52dce729;
		high ^= second.high; high = Math::RotateLeft(high, 31); high += low; high = high*5 + 0x38495ab5;
		return *this;
	}
	friend constexpr Hash128 operator+(Hash128 first, Hash128 second) {
		return first.operator+=(second);
	}
};
constexpr Hash128 operator ""_h128(char const* p, size_t s) { return Hash128{std::string_view{p, s}, 0, 0}; };

#undef L_CASE_TAIL_SHIFT
