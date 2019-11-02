#pragma once
#include <string>
#include "Serialization/ByteUtility.h"
#include "Serialization/Serializer.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	template<typename T>
	struct TPrimitiveSerializer : public ISerializer {
		static_assert(sizeof(T) < UINT32_MAX, "Serializing primitive types that are larger than UINT32_MAX is not supported");

		virtual bool SerializeBinary(Reflection::TypeInfo const& Info, void const* Data, std::ostream& Stream) const override {
			BlockSizeType const BlockSize = sizeof(T);
			WriteLE(&BlockSize, Stream);
			WriteLE(reinterpret_cast<T const*>(Data), Stream);
			return Stream.good();
		}

		virtual bool DeserializeBinary(Reflection::TypeInfo const& Info, void* Data, std::istream& Stream) const override {
			BlockSizeType BlockSize = 0;
			ReadLE(&BlockSize, Stream);
			if (BlockSize == sizeof(T)) {
				ReadLE(reinterpret_cast<T*>(Data), Stream);
				return Stream.good();
			} else {
				Stream.seekg(Stream.tellg() + std::streamoff{BlockSize});
				return false;
			}
		}
	};

	/** Strings are the only primitive with a variable size, so their serialization is more complex */
	template<typename T>
	struct TPrimitiveSerializer<std::basic_string<T>> : public ISerializer {
		using StringType = std::basic_string<T>;
		using CharacterSizeType = uint8_t;
		using StringSizeType = uint16_t;

		virtual bool SerializeBinary(Reflection::TypeInfo const& Info, void const* Data, std::ostream& Stream) const override {
			ScopedDataBlockWrite const ScopedWrite{Stream};
			StringType const& String = Cast(Data);

			CharacterSizeType const CharacterSize = sizeof(T);
			WriteLE(&CharacterSize, Stream);

			size_t const RawStringSize = String.size();
			if (RawStringSize > std::numeric_limits<StringSizeType>::max()) {
				//Strings larger than the max value of a StringSizeType are not supported.
				return false;
			}
			StringSizeType const StringSize = StringSizeType(RawStringSize);
			WriteLE(&StringSize, Stream);

			for(T const& Character : String) {
				WriteLE(&Character, Stream);
			}

			return Stream.good();
		}

		virtual bool DeserializeBinary(Reflection::TypeInfo const& Info, void* Data, std::istream& Stream) const override {
			ScopedDataBlockRead const ScopedRead{Stream};

			constexpr uint8_t MinDataBlockSize = sizeof(CharacterSizeType) + sizeof(StringSizeType);
			if (ScopedRead.GetRemainingSize() < MinDataBlockSize) {
				//Block size is not large enough to contain a string
				return false;
			}

			CharacterSizeType CharacterSize = 0;
			ReadLE(&CharacterSize, Stream);
			if(CharacterSize != sizeof(T)) {
				//Character size mismatch, cannot read string
				return false;
			}

			StringSizeType StringSize = 0;
			ReadLE(&StringSize, Stream);
			if (ScopedRead.GetRemainingSize() != (StringSize * CharacterSize)) {
				//Block size does not match what will be read for the string
				return false;
			}

			StringType& String = Cast(Data);
			String.resize(StringSize);
			for(size_t Index = 0; Index < StringSize; ++Index) {
				ReadLE(&String[Index], Stream);
			}

			return Stream.good();
		}

	private:
		static constexpr StringType& Cast(void* Data) { return *static_cast<StringType*>(Data); }
		static constexpr StringType const& Cast(void const* Data) { return *static_cast<StringType const*>(Data); }
	};
}
