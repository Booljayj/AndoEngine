#pragma once
#include <cstddef>
#include <utility>
#include <vector>
#include "Reflection/TypeResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct MapTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Map;

		/** The type of the keys in the map */
		TypeInfo const* keyType = nullptr;
		/** The type of the values in the map */
		TypeInfo const* valueType = nullptr;

		MapTypeInfo() = delete;
		MapTypeInfo(
			Hash128 inID, CompilerDefinition inDef,
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			TypeInfo const* inKeyType, TypeInfo const* inValueType
		);
		virtual ~MapTypeInfo() = default;

		/** Get the number of entries in this map */
		virtual size_t GetCount(void const* instance) const = 0;

		/** Get a vector of all entry pairs in this map */
		virtual void GetPairs(void* instance, std::vector<std::pair<void const*, void*>>& outPairs) const = 0;
		virtual void GetPairs(void const* instance, std::vector<std::pair<void const*, void const*>>& outPairs) const = 0;

		/** Find the value for a key. Returns nullptr if the key is not in the map */
		virtual void* Find(void* instance, void const* key) const = 0;
		virtual void const* Find(void const* instance, void const* key) const = 0;

		/** Remove all entries from the map */
		virtual void Clear(void* instance) const = 0;

		/** Remove the entry corresponding to a particular key from the map. Returns true if successful. */
		virtual bool RemoveEntry(void* instance, void const* key) const = 0;
		/** Add a new entry to the map with the specified value. If the key is already in the map, nothing will happen and will return false. If value is nullptr, the new value will be default constructed */
		virtual bool InsertEntry(void* instance, void const* key, void const* value) const = 0;
	};

	//============================================================
	// Templates

	template<typename MapType, typename KeyType, typename ValueType>
	struct TMapTypeInfo : public MapTypeInfo {
		TMapTypeInfo(char const* inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer)
		: MapTypeInfo(
			TypeResolver<MapType>::GetID(), GetCompilerDefinition<MapType>(),
			inDescription, inFlags, inSerializer,
			TypeResolver<KeyType>::Get(), TypeResolver<ValueType>::Get())
		{}

		STANDARD_TYPEINFO_METHODS(MapType)

		static constexpr KeyType const& CastKey(void const* key) { return *static_cast<KeyType const*>(key); }
		static constexpr ValueType const& CastValue(void const* value) { return *static_cast<ValueType const*>(value); }

		virtual size_t GetCount(void const* instance) const final { return Cast(instance).size(); }

		virtual void GetPairs(void* instance, std::vector<std::pair<void const*, void*>>& outPairs) const final {
			outPairs.clear();
			for (auto& iter : Cast(instance)) {
				outPairs.push_back(std::make_pair(&iter.first, &iter.second));
			}
		}
		virtual void GetPairs(void const* instance, std::vector<std::pair<void const*, void const*>>& outPairs) const final {
			outPairs.clear();
			for (auto const& iter : Cast(instance)) {
				outPairs.push_back(std::make_pair(&iter.first, &iter.second));
			}
		}

		virtual void* Find(void* instance, void const* key) const final {
			auto const iter = Cast(instance).find(CastKey(key));
			if (iter != Cast(instance).end()) return &(*iter);
			else return nullptr;
		}
		virtual void const* Find(void const* instance, void const* key) const final {
			auto const iter = Cast(instance).find(CastKey(key));
			if (iter != Cast(instance).end()) return &(*iter);
			else return nullptr;
		}

		virtual void Clear(void* instance) const final { Cast(instance).clear(); }

		virtual bool RemoveEntry(void* instance, void const* key) const final {
			size_t removedCount = Cast(instance).erase(CastKey(key));
			return removedCount > 0;
		}
		virtual bool InsertEntry(void* instance, void const* key, void const* value) const final {
			if (value) return Cast(instance).insert(std::make_pair(CastKey(key), CastValue(value))).second;
			else return Cast(instance).insert(std::make_pair(CastKey(key), ValueType{})).second;
		}
	};
}
