#include "Engine/SmartPointers.h"
#include "Engine/Logging.h"

LOG_CATEGORY(Serialization, Info);

void Archive::LogMissingReferenceError(ExternalObjectIdentifier identifier, Reflection::StructTypeInfo const& base) {
	LOG(Serialization, Error, "Unable to find reference {} of type {}", identifier, base.GetName());
}

void Archive::LogUnknownTypeError(ExternalObjectIdentifier identifier, Reflection::StructTypeInfo const& base) {
	LOG(Serialization, Error, "Found object with identifier {}, but cannot determine if the object is type {}", identifier, base.GetName());
}

void Archive::LogMismatchedTypeError(ExternalObjectIdentifier identifier, Reflection::StructTypeInfo const& derived, Reflection::StructTypeInfo const& base) {
	LOG(Serialization, Error, "Found object with identifier {}, but the object is of type {} which does not match the reference of type {}", identifier, derived.GetName(), base.GetName());
}
