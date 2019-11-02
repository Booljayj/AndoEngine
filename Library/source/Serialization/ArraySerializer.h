#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct ArrayTypeInfo;
}

namespace Serialization {
	struct ArraySerializer : public ISerializer {
		bool SerializeBinary(Reflection::TypeInfo const& Info, void const* Data, std::ostream& Stream) const final;
		bool DeserializeBinary(Reflection::TypeInfo const& Info, void* Data, std::istream& Stream) const final;
	};
	static const ArraySerializer DefaultArraySerializer{};
}
