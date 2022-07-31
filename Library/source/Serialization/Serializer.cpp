#include "Serialization/Serializer.h"
#include "Engine/Reflection.h"
#include "Serialization/ArraySerializer.h"
#include "Serialization/StructSerializer.h"

bool Serialization::CanSerializeType(Reflection::TypeInfo const& info) {
	return false;
}
bool Serialization::ShouldSerializeType(Reflection::TypeInfo const& info) {
	return CanSerializeType(info) && (info.flags.Has(Reflection::ETypeFlags::Serializable));
}

bool Serialization::SerializeTypeBinary(Reflection::TypeInfo const& info, void const* data, std::ostream& stream) {
	return false;
}

bool Serialization::DeserializeTypeBinary(Reflection::TypeInfo const& info, void* data, std::istream& stream) {
	return false;
}
