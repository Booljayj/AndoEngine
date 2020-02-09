#include "Serialization/Serializer.h"
#include "Serialization/ArraySerializer.h"
#include "Serialization/StructSerializer.h"
#include "Reflection/TypeInfo.h"

bool Serialization::CanSerializeType(Reflection::TypeInfo const& info) {
	return !!info.serializer;
}
bool Serialization::ShouldSerializeType(Reflection::TypeInfo const& info) {
	return CanSerializeType(info) && (info.flags.Has(Reflection::ETypeFlags::Serializable));
}

bool Serialization::SerializeTypeBinary(Reflection::TypeInfo const& info, void const* data, std::ostream& stream) {
	return info.serializer->SerializeBinary(info, data, stream);
}

bool Serialization::DeserializeTypeBinary(Reflection::TypeInfo const& info, void* data, std::istream& stream) {
	return info.serializer->DeserializeBinary(info, data, stream);
}
