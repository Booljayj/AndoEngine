#include "Serialization/ArraySerializer.h"
#include "Reflection/ArrayTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	bool ArraySerializer::SerializeBinary(Reflection::TypeInfo const& Info, void const* Data, std::ostream& Stream) const {
		Reflection::ArrayTypeInfo const* const ArrayInfo = Info.As<Reflection::ArrayTypeInfo>();
		if (!ArrayInfo) return false;

		ScopedDataBlockWrite const ScopedWrite{Stream};

		//Write the size of the array to the stream (handles cases where array size changes)
		uint32_t const ArraySize = ArrayInfo->GetCount(Data);
		WriteLE(&ArraySize, Stream);

		//Get an array of pointers to all the elements
		std::vector<void const*> Elements;
		ArrayInfo->GetElements(Data, Elements);

		//Write a data block for each element to the stream
		for (uint32_t Index = 0; Index < Elements.size(); ++Index) {
			SerializeTypeBinary(*ArrayInfo->elementType, Elements[Index], Stream);
		}
		return Stream.good();
	}

	bool ArraySerializer::DeserializeBinary(Reflection::TypeInfo const& Info, void* Data, std::istream& Stream) const {
		Reflection::ArrayTypeInfo const* const ArrayInfo = Info.As<Reflection::ArrayTypeInfo>();
		if (!ArrayInfo) return false;

		constexpr uint8_t NestedBlockHeaderSize = sizeof(BlockSizeType);

		ScopedDataBlockRead const ScopedRead{Stream};

		//Get the serialized size of the array from the data block
		uint32_t SerializedArraySize = 0;
		ReadLE(&SerializedArraySize, Stream);

		//Set the array size to match what we just read, if possible
		if (!ArrayInfo->isFixedSize) ArrayInfo->Resize(Data, SerializedArraySize);

		//Get an array of pointers to all the elements
		std::vector<void*> Elements;
		ArrayInfo->GetElements(Data, Elements);

		//Read a data block for each element from the stream
		bool bArrayReadSuccessful = true;
		for (uint32_t Index = 0; Index < Elements.size(); ++Index) {
			if (Index < SerializedArraySize && ScopedRead.GetRemainingSize() >= NestedBlockHeaderSize) {
				bool const bElementReadSuccessful = DeserializeTypeBinary(*ArrayInfo->elementType, Elements[Index], Stream);
				bArrayReadSuccessful &= bElementReadSuccessful;
			}
			else
			{
				break;
			}
		}

		return Stream.good();
	}
}
