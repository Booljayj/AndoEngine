#include "Engine/Reflection.h"
#include "Serialization/ArraySerializer.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	bool ArraySerializer::SerializeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream) const {
		Reflection::ArrayTypeInfo const* const arrayType = type.As<Reflection::ArrayTypeInfo>();
		if (!arrayType) return false;

		ScopedDataBlockWrite const scopedWrite{stream};

		//Write the size of the array to the stream (handles cases where array size changes)
		uint32_t const arraySize = arrayType->GetCount(data);
		WriteLE(&arraySize, stream);

		//Get an array of pointers to all the elements
		std::vector<void const*> elements;
		arrayType->GetElements(data, elements);

		//Write a data block for each element to the stream
		for (uint32_t index = 0; index < elements.size(); ++index) {
			SerializeTypeBinary(*arrayType->elementType, elements[index], stream);
		}
		return stream.good();
	}

	bool ArraySerializer::DeserializeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream) const {
		Reflection::ArrayTypeInfo const* const arrayType = type.As<Reflection::ArrayTypeInfo>();
		if (!arrayType) return false;

		constexpr uint8_t nestedBlockHeaderSize = sizeof(BlockSizeType);

		ScopedDataBlockRead const scopedRead{stream};

		//Get the serialized size of the array from the data block
		uint32_t serializedArraySize = 0;
		ReadLE(&serializedArraySize, stream);

		//Set the array size to match what we just read, if possible
		arrayType->Resize(data, serializedArraySize);

		//Get an array of pointers to all the elements
		std::vector<void*> elements;
		arrayType->GetElements(data, elements);

		//Read a data block for each element from the stream
		bool arrayReadSuccessful = true;
		for (uint32_t index = 0; index < elements.size(); ++index) {
			if (index < serializedArraySize && scopedRead.GetRemainingSize() >= nestedBlockHeaderSize) {
				bool const elementReadSuccessful = DeserializeTypeBinary(*arrayType->elementType, elements[index], stream);
				arrayReadSuccessful &= elementReadSuccessful;
			} else {
				break;
			}
		}

		return stream.good();
	}
}
