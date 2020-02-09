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
		template<uint32_t Size, bool IsForward>
		struct ByteSerializer;

		template<uint32_t Size>
		struct ByteSerializer<Size, true> {
			static inline void Write(char const* data, std::ostream& stream) {
				stream.write(data, Size);
			}
			static inline void Read(char* data, std::istream& stream) {
				stream.read(data, Size);
			}
		};
		template<uint32_t Size>
		struct ByteSerializer<Size, false> {
			static constexpr size_t Last = Size - 1;
			static inline void Write(char const* data, std::ostream& stream) {
				char reversedData[Size];
				for (uint8_t index = 0; index < Size; ++index) {
					reversedData[Last - index] = data[index];
				}
				stream.write(reversedData, Size);
			}
			static inline void Read(char* data, std::istream& stream) {
				char reversedData[Size];
				stream.read(reversedData, Size);
				for (uint8_t index = 0; index < Size; ++index ) {
					data[index] = reversedData[Last - index];
				}
			}
		};
	}

	/** Functions which implement serialization routines that read and write in little-endian format */
	template<typename T>
	void WriteLE(T const* data, std::ostream& stream) {
		ByteSerializer<sizeof(T), IsPlatformLittleEndian()>::Write(reinterpret_cast<char const*>(data), stream);
	}

	template<typename T>
	void ReadLE(T* data, std::istream& stream) {
		ByteSerializer<sizeof(T), IsPlatformLittleEndian()>::Read(reinterpret_cast<char*>(data), stream);
	}
}
