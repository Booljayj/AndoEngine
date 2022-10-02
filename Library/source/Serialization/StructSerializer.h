#pragma once
#include "Serialization/Serializer.h"

namespace Reflection {
	struct StructTypeInfo;
	struct VariableInfo;
}

namespace Serialization {
	struct StructSerializer : public ISerializer {
		bool SerializeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream) const final;
		bool DeserializeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream) const final;

	private:
		static bool ShouldWriteVariable(Reflection::VariableInfo const& variable);
		static bool ShouldReadVariable(Reflection::VariableInfo const& variable);
		static bool ShouldSerializeVariable(Reflection::VariableInfo const& variable);
		static void WriteVariableIdentifier(Reflection::VariableInfo const& variableInfo, std::ostream& stream);
		static Reflection::VariableInfo const* ReadVariableIdentifier(Reflection::StructTypeInfo const& structType, std::istream& stream);
	};
	static const StructSerializer defaultStructSerializer{};
}
