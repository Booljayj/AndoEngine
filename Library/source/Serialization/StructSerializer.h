#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct StructTypeInfo;
	struct VariableInfo;
}

namespace Serialization {
	struct StructSerializer : public ISerializer {
		bool SerializeBinary(Reflection::TypeInfo const& Info, void const* Data, std::ostream& Stream) const final;
		bool DeserializeBinary(Reflection::TypeInfo const& Info, void* Data, std::istream& Stream) const final;

	private:
		static void WriteVariableIdentifier(Reflection::VariableInfo const& VariableInfo, std::ostream& Stream);
		static Reflection::VariableInfo const* ReadVariableIdentifier(Reflection::StructTypeInfo const& Info, std::istream& Stream );
	};
	static const StructSerializer DefaultStructSerializer{};
}
