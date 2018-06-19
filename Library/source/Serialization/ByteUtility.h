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
		template<uint32_t SIZE, bool FORWARD>
		struct ByteSerializer;

		template<uint32_t SIZE>
		struct ByteSerializer<SIZE, true>
		{
			static inline void Write( char const* Data, std::ostream& Stream ) {
				Stream.write( Data, SIZE );
			}
			static inline void Read( char* Data, std::istream& Stream ) {
				Stream.read( Data, SIZE );
			}
		};
		template<uint32_t SIZE>
		struct ByteSerializer<SIZE, false>
		{
			static constexpr size_t LAST = SIZE - 1;
			static inline void Write( char const* Data, std::ostream& Stream ) {
				char ReversedData[SIZE];
				for( uint8_t Index = 0; Index < SIZE; ++Index ) {
					ReversedData[LAST - Index] = Data[Index];
				}
				Stream.write( ReversedData, SIZE );
			}
			static inline void Read( char* Data, std::istream& Stream ) {
				char ReversedData[SIZE];
				Stream.read( ReversedData, SIZE );
				for( uint8_t Index = 0; Index < SIZE; ++Index ) {
					Data[Index] = ReversedData[LAST - Index];
				}
			}
		};
	}

	/** Functions which implement serialization routines that read and write in little-endian format */
	template<typename T>
	void WriteLE( void const* Data, std::ostream& Stream ) {
		ByteSerializer<sizeof(T), IsPlatformLittleEndian()>::Write( static_cast<char const*>( Data ), Stream );
	}

	template<typename T>
	void ReadLE( void* Data, std::istream& Stream ) {
		ByteSerializer<sizeof(T), IsPlatformLittleEndian()>::Read( static_cast<char*>( Data ), Stream );
	}
}
