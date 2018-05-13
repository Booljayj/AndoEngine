#pragma once
#include <ostream>
#include <istream>

namespace Serialization {
	/** True if the current platform is little-endian */
	constexpr inline bool IsPlatformLittleEndian() {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		return true;
#else
		return false;
#endif
	}

	namespace {
		/** Template for structs which define byte-order serialization routines */
		template<size_t SIZE, bool ORDERED>
		struct ByteSerializer;

		template<size_t SIZE>
		struct ByteSerializer<SIZE, true>
		{
			static inline void Write( char const* Data, std::ostream& Stream ) {
				Stream.write( Data, SIZE );
			}
			static inline void Read( char* Data, std::istream& Stream ) {
				Stream.read( Data, SIZE );
			}
		};
		template<size_t SIZE>
		struct ByteSerializer<SIZE, false>
		{
			static inline void Write( char const* Data, std::ostream& Stream ) {
				char ReversedData[SIZE];
				for( uint8_t Index = 0; Index < SIZE; ++Index ) {
					ReversedData[7-Index] = Data[Index];
				}
				Stream.write( ReversedData, SIZE );
			}
			static inline void Read( char* Data, std::istream& Stream ) {
				char ReversedData[SIZE];
				Stream.read( ReversedData, SIZE );
				for( uint8_t Index = 0; Index < SIZE; ++Index ) {
					Data[Index] = ReversedData[7-Index];
				}
			}
		};
	}

	/** Struct which implements serialization routines that read and write in little-endian format */
	template<size_t SIZE>
	struct LittleEndianByteSerializer
	{
		static inline void Write( char const* Data, std::ostream& Stream ) {
			ByteSerializer<SIZE, IsPlatformLittleEndian()>::Write( Data, Stream );
		}
		static inline void Read( char* Data, std::istream& Stream ) {
			ByteSerializer<SIZE, IsPlatformLittleEndian()>::Read( Data, Stream );
		}
	};
}
