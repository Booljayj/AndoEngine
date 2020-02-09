#include "Serialization/StructSerializer.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	bool StructSerializer::SerializeBinary(Reflection::TypeInfo const& Info, void const* Data, std::ostream& Stream) const {
		Reflection::StructTypeInfo const* const StructInfo = Info.As<Reflection::StructTypeInfo>();
		if (!StructInfo) return false;

		ScopedDataBlockWrite const ScopedWrite{Stream};

		void const* DefaultData = StructInfo->Default;

		//Write an identifier and a data block for each non-default variable in the struct
		Reflection::StructTypeInfo const* CurrentStructInfo = StructInfo;
		while (CurrentStructInfo) {
			for (Reflection::VariableInfo const* MemberVariable : CurrentStructInfo->Member.Variables) {
				if (ShouldSerializeType(*MemberVariable->Type)) {
					//Get a pointer to the value we want to serialize, and a pointer to the default version of that value
					void const* VariablePointer = MemberVariable->GetImmutableValuePointer(Data);
					void const* DefaultVariablePointer = MemberVariable->GetImmutableValuePointer(DefaultData);

					//Compare the variable to the default. If the value is the same as the default, then we don't need to write anything
					if (!MemberVariable->Type->Equal(VariablePointer, DefaultVariablePointer)) {
						WriteVariableIdentifier(*MemberVariable, Stream);
						SerializeTypeBinary(*MemberVariable->Type, VariablePointer, Stream);
					}
				}
			}
			CurrentStructInfo = CurrentStructInfo->BaseTypeInfo;
		}

		return Stream.good();
	}

	bool StructSerializer::DeserializeBinary(Reflection::TypeInfo const& Info, void* Data, std::istream& Stream) const {
		Reflection::StructTypeInfo const* const StructInfo = Info.As<Reflection::StructTypeInfo>();
		if (!StructInfo) return false;

		constexpr uint8_t NestedBlockHeaderSize = sizeof(Hash32) + sizeof(BlockSizeType);
		ScopedDataBlockRead const ScopedRead{Stream};

		while (Stream.good() && ScopedRead.GetRemainingSize() >= NestedBlockHeaderSize) {
			//Get the next variable to deserialize
			Reflection::VariableInfo const* MemberVariable = ReadVariableIdentifier(*StructInfo, Stream);
			if (!Stream.good()) return false; //Make sure the read was successful

			//If the struct has a variable with this ID, and it can be serialized, attempt to deserialize it.
			if (MemberVariable && ShouldSerializeType(*MemberVariable->Type)) {
				void* VariablePointer = MemberVariable->GetMutableValuePointer(Data);
				bool const bSuccess = DeserializeTypeBinary(*MemberVariable->Type, VariablePointer, Stream);
				if (!bSuccess) return false; //@todo Report an error just for this variable instead of returning false

			} else {
				ScopedRead.SkipNestedBlock();
			}
		}

		return Stream.good();
	}

	void StructSerializer::WriteVariableIdentifier(Reflection::VariableInfo const& VariableInfo, std::ostream& Stream) {
		decltype(Hash32::hash) const NameHashValue = VariableInfo.NameHash.hash;
		WriteLE(&(NameHashValue), Stream);
	}

	Reflection::VariableInfo const* StructSerializer::ReadVariableIdentifier(Reflection::StructTypeInfo const& StructInfo, std::istream& Stream) {
		decltype(Hash32::hash) NameHashValue = 0;
		ReadLE(&NameHashValue, Stream);

		//Walk up the chain of base classes, searching for a variable with the correct name hash.
		Reflection::StructTypeInfo const* CurrentStructInfo = &StructInfo;
		while (CurrentStructInfo) {
			if (Reflection::VariableInfo const* FoundInfo = CurrentStructInfo->Member.Variables.Find(Hash32{NameHashValue})) {
				return FoundInfo;
			}
			CurrentStructInfo = CurrentStructInfo->BaseTypeInfo;
		}
		return nullptr;
	}
}
