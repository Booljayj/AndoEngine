#pragma once
#include <cstddef>
#include <utility>
#include <vector>
#include "Reflection/TypeResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct MapTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Map;

		/** The type of the keys in the map */
		TypeInfo const* KeyTypeInfo = nullptr;
		/** The type of the values in the map */
		TypeInfo const* ValueTypeInfo = nullptr;

		MapTypeInfo() = delete;
		MapTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TypeInfo const* InKeyTypeInfo, TypeInfo const* InValueTypeInfo
		);
		virtual ~MapTypeInfo() = default;

		/** Get the number of entries in this map */
		virtual size_t GetCount(void const* Instance) const = 0;

		/** Get a vector of all entry pairs in this map */
		virtual void GetPairs(void* Instance, std::vector<std::pair<void const*, void*>>& OutPairs) const = 0;
		virtual void GetPairs(void const* Instance, std::vector<std::pair<void const*, void const*>>& OutPairs) const = 0;

		/** Find the value for a key. Returns nullptr if the key is not in the map */
		virtual void* Find(void* Instance, void const* Key) const = 0;
		virtual void const* Find(void const* Instance, void const* Key) const = 0;

		/** Remove all entries from the map */
		virtual void Clear(void* Instance) const = 0;

		/** Remove the entry corresponding to a particular key from the map. Returns true if successful. */
		virtual bool RemoveEntry(void* Instance, void const* Key) const = 0;
		/** Add a new entry to the map with the specified value. If the key is already in the map, nothing will happen and will return false. If Value is nullptr, the new value will be default constructed */
		virtual bool InsertEntry(void* Instance, void const* Key, void const* Value) const = 0;
	};

	//============================================================
	// Templates

	template<typename KeyType, typename ValueType, typename MapType>
	struct TMapTypeInfo : public MapTypeInfo {
		TMapTypeInfo(char const* InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: MapTypeInfo(
			TypeResolver<MapType>::GetID(), GetCompilerDefinition<MapType>(),
			InDescription, InFlags, InSerializer,
			TypeResolver<KeyType>::Get(), TypeResolver<ValueType>::Get())
		{}

		STANDARD_TYPEINFO_METHODS(MapType)

		static constexpr KeyType const& CastKey(void const* Key) { return *static_cast<KeyType const*>(Key); }
		static constexpr ValueType const& CastValue(void const* Value) { return *static_cast<ValueType const*>(Value); }

		virtual size_t GetCount(void const* Instance) const final { return Cast(Instance).size(); }

		virtual void GetPairs(void* Instance, std::vector<std::pair<void const*, void*>>& OutPairs) const final {
			OutPairs.clear();
			for (auto& Iter : Cast(Instance)) {
				OutPairs.push_back(std::make_pair(&Iter.first, &Iter.second));
			}
		}
		virtual void GetPairs(void const* Instance, std::vector<std::pair<void const*, void const*>>& OutPairs) const final {
			OutPairs.clear();
			for (auto const& Iter : Cast(Instance)) {
				OutPairs.push_back(std::make_pair(&Iter.first, &Iter.second));
			}
		}

		virtual void* Find(void* Instance, void const* Key) const final {
			auto const Iter = Cast(Instance).find(CastKey(Key));
			if (Iter != Cast(Instance).end()) return &(*Iter);
			else return nullptr;
		}
		virtual void const* Find(void const* Instance, void const* Key) const final {
			auto const Iter = Cast(Instance).find(CastKey(Key));
			if (Iter != Cast(Instance).end()) return &(*Iter);
			else return nullptr;
		}

		virtual void Clear(void* Instance) const final { Cast(Instance).clear(); }

		virtual bool RemoveEntry(void* Instance, void const* Key) const final {
			size_t RemovedCount = Cast(Instance).erase(CastKey(Key));
			return RemovedCount > 0;
		}
		virtual bool InsertEntry(void* Instance, void const* Key, void const* Value) const final {
			if (Value) return Cast(Instance).insert(std::make_pair(CastKey(Key), CastValue(Value))).second;
			else return Cast(Instance).insert(std::make_pair(CastKey(Key), ValueType{})).second;
		}
	};
}
