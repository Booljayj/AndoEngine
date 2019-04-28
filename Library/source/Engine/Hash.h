#pragma once
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <boost/endian/conversion.hpp>
#include "Engine/Math.h"

//String hash types. These store hashes in specialized types which encapsulate construction, combining, and
// storing hashes of various lengths.

//Stable in this context means that the hash will not change between program executions or between platforms,
// which makes these hashes suitable for long-term identifiers.
//The input can either be a utf8 string or an ascii string, which give different results. Providing proper
// input is left to the caller, so ensure that if you are matching a user input string hash to a hardcoded one,
// the string encoding is the same.
//Because these hashes are stable, the implementation CANNOT change after it has been used for any kind of
// long-term data storage. Bugs or changes will likely require a full version change and some data migration
// to update any hashes that are being used.

/** A 32-bit string hash */
struct Hash32 {
private:
	static constexpr uint32_t C1 = 0xcc9e2d51UL;
	static constexpr uint32_t C2 = 0x1b873593UL;
	static constexpr size_t BlockSize = sizeof( uint32_t );

	static constexpr uint32_t Mix( uint32_t K ) {
		K ^= K >> 16;
		K *= 0x85ebca6b;
		K ^= K >> 13;
		K *= 0xc2b2ae35;
		K ^= K >> 16;
		return K;
	}

public:
	uint32_t Hash;

	constexpr Hash32() : Hash(0) {}
	explicit constexpr Hash32( uint32_t InValue ) : Hash( InValue ) {}
	constexpr Hash32( const Hash32& Other ) : Hash( Other.Hash ) {}
	/** Primary constructor, which creates the hash for some string */
	constexpr Hash32( std::string_view String, uint32_t Seed = 0 ) : Hash( Seed ) {
		// Calculate a 32-bit Murmur3 hash for the string view
		if( String.data() != nullptr && String.size() > 0) {
			size_t const NumBlocks = String.size() / BlockSize;
			uint32_t const* const Blocks = (uint32_t const*)(String.data());
			uint8_t const* const Tail = (uint8_t const*)(String.data() + NumBlocks*BlockSize);

			// Body
			for( size_t BlockIndex = 0; BlockIndex < NumBlocks; ++BlockIndex ) {
				uint32_t Value = boost::endian::little_to_native( Blocks[BlockIndex] );
				Value *= C1; Value = Math::RotateLeft( Value, 15 ); Value *= C2;
				Hash ^= Value; Hash = Math::RotateLeft( Hash, 13 ); Hash = Hash*5 + 0xe6546b64;
			}

			// Tail
			uint32_t Value = 0;
			switch( String.size() & 3 ) {
			case 3: Value ^= Tail[2] << 16;
			case 2: Value ^= Tail[1] << 8;
			case 1: Value ^= Tail[0];
					Value *= C1; Value = Math::RotateLeft( Value, 15 ); Value *= C2;
					Hash ^= Value;
			};

			// Finalization
			Hash ^= String.size();
			Hash = Mix( Hash );
		}
	}

	constexpr bool operator==( const Hash32& Other ) const { return Hash == Other.Hash; }
	constexpr bool operator!=( const Hash32& Other ) const { return !operator==(Other); }

	constexpr Hash32& operator+=( Hash32 Second ) {
		Second.Hash *= C1; Second.Hash = Math::RotateLeft( Second.Hash, 15 ); Second.Hash *= C2;
		Hash ^= Second.Hash; Hash = Math::RotateLeft( Hash, 13 ); Hash = Hash*5 + 0xe6546b64;
		return *this;
	}
	friend constexpr Hash32 operator+( Hash32 First, Hash32 Second ) {
		return First.operator+=( Second );
	}
};

/** A 64-bit string hash */
struct Hash64 {
private:
	static constexpr uint64_t C1 = 0x87c37b91114253d5ULL;
	static constexpr uint64_t C2 = 0x4cf5ad432745937fULL;
	static constexpr size_t BlockSize = sizeof( uint64_t );

	static constexpr uint64_t Mix( uint64_t K ) {
		K ^= K >> 33;
		K *= 0xff51afd7ed558ccdULL;
		K ^= K >> 33;
		K *= 0xc4ceb9fe1a85ec53ULL;
		K ^= K >> 33;
		return K;
	}

public:
	uint64_t Hash;

	constexpr Hash64() : Hash(0) {}
	explicit constexpr Hash64( uint64_t InValue ) : Hash( InValue ) {}
	constexpr Hash64( const Hash64& Other ) : Hash( Other.Hash ) {}
	/** Primary constructor, which creates the hash for some string */
	constexpr Hash64( std::string_view String, uint64_t Seed = 0 ) : Hash( Seed ) {
		// Calculate a 64-bit Murmur3 hash for a string view
		//Not standard, extrapolated from the 32-bit and 128-bit hash implementations
		if( String.data() != nullptr && String.size() > 0 ) {
			size_t const NumBlocks = String.size() / BlockSize;
			uint64_t const* const Blocks = (uint64_t const*)(String.data());
			uint8_t const* const Tail = (uint8_t const*)(String.data() + (NumBlocks*BlockSize));

			// Body
			for( size_t BlockIndex = 0; BlockIndex < NumBlocks; BlockIndex++) {
				uint64_t Value = boost::endian::little_to_native( Blocks[BlockIndex] );
				Value *= C1; Value = Math::RotateLeft( Value, 31 ); Value *= C2;
				Hash ^= Value; Hash = Math::RotateLeft( Hash, 27 ); Hash = Hash*5 + 0x52dce729;
			}

			// Tail
			uint64_t Value = 0;
			switch( String.size() & 7 ) {
			case  7: Value ^= ((uint64_t)Tail[6]) << 48;
			case  6: Value ^= ((uint64_t)Tail[5]) << 40;
			case  5: Value ^= ((uint64_t)Tail[4]) << 32;
			case  4: Value ^= ((uint64_t)Tail[3]) << 24;
			case  3: Value ^= ((uint64_t)Tail[2]) << 16;
			case  2: Value ^= ((uint64_t)Tail[1]) << 8;
			case  1: Value ^= ((uint64_t)Tail[0]) << 0;
				Value *= C1; Value = Math::RotateLeft( Value, 31 ); Value *= C2;
				Hash ^= Value;
			};

			// Finalization
			Hash ^= String.size();
			Hash = Mix( Hash );
		}
	}

	constexpr bool operator==( const Hash64& Other ) const { return Hash == Other.Hash; }
	constexpr bool operator!=( const Hash64& Other ) const { return !operator==(Other); }

	constexpr Hash64& operator+=( Hash64 Second ) {
		Second.Hash *= C1; Second.Hash = Math::RotateLeft( Second.Hash, 31 ); Second.Hash *= C2;
		Hash ^= Second.Hash; Hash = Math::RotateLeft( Hash, 27 ); Hash = Hash*5 + 0x52dce729;
		return *this;
	}
	friend constexpr Hash64 operator+( Hash64 First, Hash64 Second ) {
		return First.operator+=( Second );
	}
};

/** A 128-bit string hash */
struct Hash128 {
private:
	static constexpr uint64_t C1 = 0x87c37b91114253d5ULL;
	static constexpr uint64_t C2 = 0x4cf5ad432745937fULL;
	static constexpr size_t BlockSize = sizeof( uint64_t );

	static constexpr uint64_t Mix( uint64_t K ) {
		K ^= K >> 33;
		K *= 0xff51afd7ed558ccdULL;
		K ^= K >> 33;
		K *= 0xc4ceb9fe1a85ec53ULL;
		K ^= K >> 33;
		return K;
	}

public:
	uint64_t Low;
	uint64_t High;

	constexpr Hash128() : Low(0), High(0) {}
	explicit constexpr Hash128( uint64_t InValue ) : Low( InValue ), High( InValue ) {}
	explicit constexpr Hash128( uint64_t InLow, uint64_t InHigh ) : Low( InLow ), High( InHigh ) {}
	constexpr Hash128( const Hash128& Other ) : Low( Other.Low ), High( Other.High ) {}
	/** Primary constructor, which creates the hash for some string */
	constexpr Hash128( std::string_view String, uint64_t LowSeed = 0, uint64_t HighSeed = 0 ) : Low( LowSeed ), High( HighSeed ) {
		// Calculate a 128-bit Murmur3 hash for a string view
		if( String.data() != nullptr && String.size() > 0 ) {
			uint64_t const NumBlockPairs = String.size() / (BlockSize*2);
			uint64_t const* const Blocks = (uint64_t const*)(String.data());
			uint8_t const* const Tail = (uint8_t const*)(String.data() + (NumBlockPairs*BlockSize*2));

			// Body
			for( size_t BlockPairIndex = 0; BlockPairIndex < NumBlockPairs; BlockPairIndex++) {
				uint64_t Value1 = boost::endian::little_to_native( Blocks[BlockPairIndex * 2 + 0] );
				uint64_t Value2 = boost::endian::little_to_native( Blocks[BlockPairIndex * 2 + 1] );

				Value1 *= C1; Value1 = Math::RotateLeft( Value1, 31 ); Value1 *= C2;
				Value2 *= C2; Value2 = Math::RotateLeft( Value2, 33 ); Value2 *= C1;
				Low ^= Value1; Low = Math::RotateLeft( Low, 27 ); Low += High; Low = Low*5 + 0x52dce729;
				High ^= Value2; High = Math::RotateLeft( High, 31 ); High += Low; High = High*5 + 0x38495ab5;
			}

			// Tail
			uint64_t Value1 = 0;
			uint64_t Value2 = 0;
			switch( String.size() & 15 ) {
			case 15: Value2 ^= ((uint64_t)Tail[14]) << 48;
			case 14: Value2 ^= ((uint64_t)Tail[13]) << 40;
			case 13: Value2 ^= ((uint64_t)Tail[12]) << 32;
			case 12: Value2 ^= ((uint64_t)Tail[11]) << 24;
			case 11: Value2 ^= ((uint64_t)Tail[10]) << 16;
			case 10: Value2 ^= ((uint64_t)Tail[ 9]) << 8;
			case  9: Value2 ^= ((uint64_t)Tail[ 8]) << 0;
				Value2 *= C2; Value2 = Math::RotateLeft( Value2, 33 ); Value2 *= C1;
				High ^= Value2;

			case  8: Value1 ^= ((uint64_t)Tail[ 7]) << 56;
			case  7: Value1 ^= ((uint64_t)Tail[ 6]) << 48;
			case  6: Value1 ^= ((uint64_t)Tail[ 5]) << 40;
			case  5: Value1 ^= ((uint64_t)Tail[ 4]) << 32;
			case  4: Value1 ^= ((uint64_t)Tail[ 3]) << 24;
			case  3: Value1 ^= ((uint64_t)Tail[ 2]) << 16;
			case  2: Value1 ^= ((uint64_t)Tail[ 1]) << 8;
			case  1: Value1 ^= ((uint64_t)Tail[ 0]) << 0;
				Value1 *= C1; Value1 = Math::RotateLeft( Value1, 31 ); Value1 *= C2;
				Low ^= Value1;
			};

			// Finalization
			Low ^= String.size();
			High ^= String.size();

			Low += High;
			High += Low;

			Low = Mix( Low );
			High = Mix( High );

			Low += High;
			High += Low;
		}
	}

	constexpr bool operator==( const Hash128& Other ) const { return Low == Other.Low && High == Other.High; }
	constexpr bool operator!=( const Hash128& Other ) const { return !operator==(Other); }

	constexpr Hash128& operator+=( Hash128 Second ) {
		Second.Low *= C1; Second.Low = Math::RotateLeft( Second.Low, 31 ); Second.Low *= C2;
		Second.High *= C2; Second.High = Math::RotateLeft( Second.High, 33 ); Second.High *= C1;
		Low ^= Second.Low; Low = Math::RotateLeft( Low, 27 ); Low += High; Low = Low*5 + 0x52dce729;
		High ^= Second.High; High = Math::RotateLeft( High, 31 ); High += Low; High = High*5 + 0x38495ab5;
		return *this;
	}
	friend constexpr Hash128 operator+( Hash128 First, Hash128 Second ) {
		return First.operator+=( Second );
	}
};
