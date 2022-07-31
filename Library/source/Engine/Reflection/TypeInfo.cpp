#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	//Non-public static accessor that can be used by the TypeInfo constructor
	std::deque<TypeInfo const*>& GetMutableGlobalTypeInfoCollection() {
		static std::deque<TypeInfo const*> globalCollection{};
		return globalCollection;
	}

	std::deque<TypeInfo const*> const& TypeInfo::GetGlobalTypeInfoCollection() {
		return GetMutableGlobalTypeInfoCollection();
	}

	TypeInfo const* TypeInfo::FindTypeByID(Hash128 id, uint64_t library) {
		if (library == 0) {
			for (TypeInfo const* info : GetGlobalTypeInfoCollection()) {
				if (info->id == id) return info;
			}
		} else {
			for (TypeInfo const* info : GetGlobalTypeInfoCollection()) {
				if (info->library == library && info->id == id) return info;
			}
		}
		return nullptr;
	}

	TypeInfo::TypeInfo(ETypeClassification inClassification, uint64_t inLibrary, Hash128 inID, std::string_view inName, FTypeFlags inFlags, MemoryParams inMemory)
		: classification(inClassification), library(inLibrary), id(inID), name(inName), flags(inFlags), memory(inMemory)
	{
		GetMutableGlobalTypeInfoCollection().push_back(this);
	}

	TypeInfo::~TypeInfo() {
		auto& TypeInfoCollection = GetMutableGlobalTypeInfoCollection();
		auto iter = std::find(TypeInfoCollection.begin(), TypeInfoCollection.end(), this);
		if (iter != TypeInfoCollection.end()) {
			std::iter_swap(iter, TypeInfoCollection.end() - 1);
			TypeInfoCollection.pop_back();
		}
	}
}
