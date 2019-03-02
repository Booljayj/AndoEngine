#include <iomanip>
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeUtility.h"
#include "Reflection/StructTypeInfo.h"
#include "Engine/StringID.h"

namespace Reflection {
	//Non-public static accessor that can be used by the TypeInfo constructor
	std::deque<TypeInfo const*>& GetMutableGlobalTypeInfoCollection() {
		static std::deque<TypeInfo const*> GlobalCollection{};
		return GlobalCollection;
	}

	std::deque<TypeInfo const*> const& TypeInfo::GetGlobalTypeInfoCollection() {
		return GetMutableGlobalTypeInfoCollection();
	}

	TypeInfo const* TypeInfo::FindTypeByID( sid_t UniqueID ) {
		for( TypeInfo const* Info : GetGlobalTypeInfoCollection() ) {
			if( Info && Info->UniqueID == UniqueID ) {
				return Info;
			}
		}
		return nullptr;
	}

	TypeInfo::TypeInfo(
		ETypeClassification InClassification, sid_t InUniqueID, CompilerDefinition InDefinition,
		char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer
	)
	: Classification( InClassification ), UniqueID( InUniqueID ), Definition( InDefinition )
	, Description( InDescription ), Flags( InFlags ), Serializer( InSerializer )
	{
		GetMutableGlobalTypeInfoCollection().push_back( this );
	}
}
