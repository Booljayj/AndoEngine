#include <deque>
#include "Reflection/TypeInfo.h"

namespace Reflection {
	//Non-public static accessor that can be used by the TypeInfo constructor
	std::deque<TypeInfo const*>& GetMutableGlobalTypeInfoCollection() {
		static std::deque<TypeInfo const*> GlobalCollection{};
		return GlobalCollection;
	}

	std::deque<TypeInfo const*> const& TypeInfo::GetGlobalTypeInfoCollection() {
		return GetMutableGlobalTypeInfoCollection();
	}

	TypeInfo const* TypeInfo::FindTypeByID( Hash128 UniqueID ) {
		for (TypeInfo const* Info : GetGlobalTypeInfoCollection()) {
			if (Info && Info->UniqueID == UniqueID) return Info;
		}
		return nullptr;
	}

	TypeInfo::TypeInfo(
		ETypeClassification InClassification, Hash128 InUniqueID, CompilerDefinition InDefinition,
		std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
	: Classification(InClassification), UniqueID(InUniqueID), Definition(InDefinition)
	, Description(InDescription), Flags(InFlags), Serializer(InSerializer)
	{
		GetMutableGlobalTypeInfoCollection().push_back(this);
	}
}
