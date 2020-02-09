#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct ArrayTypeInfo;
}

namespace Serialization {
	struct ArraySerializer : public ISerializer {
		bool SerializeBinary(Reflection::TypeInfo const& type, void const* data, std::ostream& stream) const final;
		bool DeserializeBinary(Reflection::TypeInfo const& type, void* data, std::istream& stream) const final;
	};
	static const ArraySerializer defaultArraySerializer{};
}
