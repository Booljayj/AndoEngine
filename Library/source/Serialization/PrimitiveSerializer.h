#pragma once
#include "Engine/STL.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/Serializer.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	template<typename T>
	struct TPrimitiveSerializer : public ISerializer {
		static_assert(sizeof(T) < std::numeric_limits<uint32_t>::max(), "Serializing primitive types that are larger than UINT32_MAX is not supported");

		virtual bool SerializeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream) const override {
			constexpr BlockSizeType BlockSize = sizeof(T);
			WriteLE(&BlockSize, stream);
			WriteLE(reinterpret_cast<T const*>(data), stream);
			return stream.good();
		}

		virtual bool DeserializeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream) const override {
			BlockSizeType blockSize = 0;
			ReadLE(&blockSize, stream);
			if (blockSize == sizeof(T)) {
				ReadLE(reinterpret_cast<T*>(data), stream);
				return stream.good();
			} else {
				stream.seekg(stream.tellg() + std::streamoff{blockSize});
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

		virtual bool SerializeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream) const override {
			ScopedDataBlockWrite const scopedWrite{stream};
			StringType const& string = Cast(data);

			constexpr CharacterSizeType const CharacterSize = sizeof(T);
			WriteLE(&CharacterSize, stream);

			size_t const rawStringSize = string.size();
			if (rawStringSize > std::numeric_limits<StringSizeType>::max()) {
				//Strings larger than the max value of a StringSizeType are not supported.
				return false;
			}
			StringSizeType const stringSize = StringSizeType(rawStringSize);
			WriteLE(&stringSize, stream);

			for(T const& character : string) {
				WriteLE(&character, stream);
			}

			return stream.good();
		}

		virtual bool DeserializeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream) const override {
			ScopedDataBlockRead const scopedRead{stream};

			constexpr uint8_t MinDataBlockSize = sizeof(CharacterSizeType) + sizeof(StringSizeType);
			if (scopedRead.GetRemainingSize() < MinDataBlockSize) {
				//Block size is not large enough to contain a string
				return false;
			}

			CharacterSizeType characterSize = 0;
			ReadLE(&characterSize, stream);
			if(characterSize != sizeof(T)) {
				//character size mismatch, cannot read string
				return false;
			}

			StringSizeType stringSize = 0;
			ReadLE(&stringSize, stream);
			if (scopedRead.GetRemainingSize() != (stringSize * characterSize)) {
				//Block size does not match what will be read for the string
				return false;
			}

			StringType& string = Cast(data);
			string.resize(stringSize);
			for(size_t Index = 0; Index < stringSize; ++Index) {
				ReadLE(&string[Index], stream);
			}

			return stream.good();
		}

	private:
		static constexpr StringType& Cast(void* data) { return *static_cast<StringType*>(data); }
		static constexpr StringType const& Cast(void const* data) { return *static_cast<StringType const*>(data); }
	};
}
