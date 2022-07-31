#include "Serialization/StructSerializer.h"
#include "Engine/Reflection.h"
#include "Serialization/ByteUtility.h"
#include "Serialization/SerializationUtility.h"

namespace Serialization {
	using namespace Reflection;

	bool StructSerializer::SerializeBinary(TypeInfo const& type, void const* data, std::ostream& stream) const {
		StructTypeInfo const* const structType = type.As<StructTypeInfo>();
		if (!structType) return false;

		ScopedDataBlockWrite const scopedWrite{stream};

		//Write an identifier and a data block for each non-default variable in the struct
		StructTypeInfo const* currentStructType = structType;
		while (currentStructType) {
			for (VariableInfo const& variable : currentStructType->variables) {
				if (ShouldSerializeVariable(variable) && ShouldSerializeType(*variable.type)) {
					//Get a pointer to the value we want to serialize, and a pointer to the default version of that value
					void const* dataValuePointer = variable.GetValuePointer(data);
					void const* defaultValuePointer = variable.GetValuePointer(structType->defaults);

					//Compare the variable to the default. If the value is the same as the default, then we don't need to write anything
					if (!variable.type->Equal(dataValuePointer, defaultValuePointer)) {
						WriteVariableIdentifier(variable, stream);
						SerializeTypeBinary(*variable.type, dataValuePointer, stream);
					}
				}
			}
			currentStructType = currentStructType->baseType;
		}

		return stream.good();
	}

	bool StructSerializer::DeserializeBinary(TypeInfo const& type, void* data, std::istream& stream) const {
		StructTypeInfo const* const structType = type.As<StructTypeInfo>();
		if (!structType) return false;

		constexpr uint8_t NestedBlockHeaderSize = sizeof(Hash32) + sizeof(BlockSizeType);
		ScopedDataBlockRead const scopedRead{stream};

		while (stream.good() && scopedRead.GetRemainingSize() >= NestedBlockHeaderSize) {
			//Get the next variable to deserialize
			VariableInfo const* variable = ReadVariableIdentifier(*structType, stream);
			if (!stream.good()) return false; //Make sure the read was successful

			//If the struct has a variable with this ID, and it can be serialized, attempt to deserialize it.
			if (variable && ShouldSerializeVariable(*variable) && ShouldSerializeType(*variable->type)) {
				void* dataValuePointer = variable->GetValuePointer(data);
				bool const success = DeserializeTypeBinary(*variable->type, dataValuePointer, stream);
				if (!success) return false; //@todo Report an error just for this variable instead of returning false

			} else {
				scopedRead.SkipNestedBlock();
			}
		}

		return stream.good();
	}

	bool StructSerializer::ShouldSerializeVariable(VariableInfo const& variable) {
		constexpr auto excludedFlags = FVariableFlags::Make(EVariableFlags::Const, EVariableFlags::NonSerialized);
		return variable.flags.HasAll(excludedFlags);
	}

	void StructSerializer::WriteVariableIdentifier(VariableInfo const& variable, std::ostream& stream) {
		Hash32 const id = variable.id;
		WriteLE(&id.hash, stream);
	}

	VariableInfo const* StructSerializer::ReadVariableIdentifier(StructTypeInfo const& structType, std::istream& stream) {
		Hash32 id;
		ReadLE(&id.hash, stream);

		//Walk up the chain of base classes, searching for a variable with the correct name hash.
		StructTypeInfo const* currentStructType = &structType;
		while (currentStructType) {
			if (VariableInfo const* foundVariable = currentStructType->FindVariable(id)) {
				return foundVariable;
			}
			currentStructType = currentStructType->baseType;
		}
		return nullptr;
	}
}
