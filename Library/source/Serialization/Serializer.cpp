#include "Serialization/Serializer.h"
#include "Serialization/ArraySerializer.h"
#include "Serialization/StructSerializer.h"
#include "Reflection/TypeInfo.h"

bool Serialization::CanSerializeType(Reflection::TypeInfo const& Info) {
	return !!Info.Serializer;
}
bool Serialization::ShouldSerializeType(Reflection::TypeInfo const& Info) {
	return CanSerializeType(Info) && (Info.Flags * Reflection::FTypeFlags::Serializable);
}

bool Serialization::SerializeTypeBinary(Reflection::TypeInfo const& Info, void const* Data, std::ostream& Stream) {
	return Info.Serializer->SerializeBinary(Info, Data, Stream);
}

bool Serialization::DeserializeTypeBinary(Reflection::TypeInfo const& Info, void* Data, std::istream& Stream) {
	return Info.Serializer->DeserializeBinary(Info, Data, Stream);
}
