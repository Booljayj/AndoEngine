#include "Reflection/TypeInfo.h"

namespace Reflection {
	//Non-public static accessor that can be used by the TypeInfo constructor
	std::deque<TypeInfo const*>& GetMutableGlobalTypeInfoCollection() {
		static std::deque<TypeInfo const*> globalCollection{};
		return globalCollection;
	}

	std::deque<TypeInfo const*> const& TypeInfo::GetGlobalTypeInfoCollection() {
		return GetMutableGlobalTypeInfoCollection();
	}

	TypeInfo const* TypeInfo::FindTypeByID(Hash128 id) {
		for (TypeInfo const* info : GetGlobalTypeInfoCollection()) {
			if (info && info->id == id) return info;
		}
		return nullptr;
	}

	TypeInfo::TypeInfo(
		ETypeClassification inClassification, Hash128 inID, CompilerDefinition inDef,
		std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer)
	: classification(inClassification), id(inID), def(inDef)
	, description(inDescription), flags(inFlags), serializer(inSerializer)
	{
		GetMutableGlobalTypeInfoCollection().push_back(this);
	}

	TypeInfo::TypeInfo(ETypeClassification inClassification, Hash128 inUniqueID, CompilerDefinition inDefinition)
	: TypeInfo(inClassification, inUniqueID, inDefinition, std::string_view{}, FTypeFlags::None, nullptr)
	{}
}
