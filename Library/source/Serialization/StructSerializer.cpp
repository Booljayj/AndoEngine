#include "Serialization/StructSerializer.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/Components/VariableInfo.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	bool StructSerializer::SerializeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream) const {
		Reflection::StructTypeInfo const* const structInfo = type.As<Reflection::StructTypeInfo>();
		if (!structInfo) return false;

		ScopedDataBlockWrite const scopedWrite{stream};

		void const* defaults = structInfo->defaults;

		//Write an identifier and a data block for each non-default variable in the struct
		Reflection::StructTypeInfo const* currentStructInfo = structInfo;
		while (currentStructInfo) {
			for (Reflection::VariableInfo const* memberVariable : currentStructInfo->members.variables) {
				if (ShouldSerializeType(*memberVariable->type)) {
					//Get a pointer to the value we want to serialize, and a pointer to the default version of that value
					void const* variablePointer = memberVariable->GetImmutableValuePointer(data);
					void const* defaultVariablePointer = memberVariable->GetImmutableValuePointer(defaults);

					//Compare the variable to the default. If the value is the same as the default, then we don't need to write anything
					if (!memberVariable->type->Equal(variablePointer, defaultVariablePointer)) {
						WriteVariableIdentifier(*memberVariable, stream);
						SerializeTypeBinary(*memberVariable->type, variablePointer, stream);
					}
				}
			}
			currentStructInfo = currentStructInfo->baseType;
		}

		return stream.good();
	}

	bool StructSerializer::DeserializeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream) const {
		Reflection::StructTypeInfo const* const structInfo = type.As<Reflection::StructTypeInfo>();
		if (!structInfo) return false;

		constexpr uint8_t NestedBlockHeaderSize = sizeof(Hash32) + sizeof(BlockSizeType);
		ScopedDataBlockRead const scopedRead{stream};

		while (stream.good() && scopedRead.GetRemainingSize() >= NestedBlockHeaderSize) {
			//Get the next variable to deserialize
			Reflection::VariableInfo const* memberVariable = ReadVariableIdentifier(*structInfo, stream);
			if (!stream.good()) return false; //Make sure the read was successful

			//If the struct has a variable with this ID, and it can be serialized, attempt to deserialize it.
			if (memberVariable && ShouldSerializeType(*memberVariable->type)) {
				void* variablePointer = memberVariable->GetMutableValuePointer(data);
				bool const success = DeserializeTypeBinary(*memberVariable->type, variablePointer, stream);
				if (!success) return false; //@todo Report an error just for this variable instead of returning false

			} else {
				scopedRead.SkipNestedBlock();
			}
		}

		return stream.good();
	}

	void StructSerializer::WriteVariableIdentifier(Reflection::VariableInfo const& variableInfo, std::ostream& stream) {
		decltype(Hash32::hash) const idValue = variableInfo.id.hash;
		WriteLE(&(idValue), stream);
	}

	Reflection::VariableInfo const* StructSerializer::ReadVariableIdentifier(Reflection::StructTypeInfo const& structInfo, std::istream& stream) {
		decltype(Hash32::hash) idValue = 0;
		ReadLE(&idValue, stream);

		//Walk up the chain of base classes, searching for a variable with the correct name hash.
		Reflection::StructTypeInfo const* currentStructInfo = &structInfo;
		while (currentStructInfo) {
			if (Reflection::VariableInfo const* foundInfo = currentStructInfo->members.variables.Find(Hash32{idValue})) {
				return foundInfo;
			}
			currentStructInfo = currentStructInfo->baseType;
		}
		return nullptr;
	}
}
