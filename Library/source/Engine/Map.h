#pragma once
#include <map>
#include <unordered_map>
#include "Engine/Core.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Archive {
	template<Concepts::ReadWritable K, Concepts::ReadWritable V>
	struct Serializer<std::map<K, V>> {
		static inline void Write(Output& archive, std::map<K, V> const& map) {
			Serializer<size_t>::Write(archive, map.size());
			for (const auto& [key, value] : map) {
				Serializer<K>::Write(archive, key);
				Serializer<V>::Write(archive, value);
			}
		}
		static inline void Read(Input& archive, std::map<K, V>& map) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);

			for (size_t index = 0; index < num; ++index) {
				K key = DefaultReadValue<K>();
				V value = DefaultReadValue<V>();

				Serializer<K>::Read(archive, key);
				Serializer<V>::Read(archive, value);

				map.emplace(key, value);
			}
		}
	};

	template<Concepts::ReadWritable K, Concepts::ReadWritable V>
	struct Serializer<std::unordered_map<K, V>> {
		static inline void Write(Output& archive, std::unordered_map<K, V> const& map) {
			Serializer<size_t>::Write(archive, map.size());
			for (const auto& [key, value] : map) {
				Serializer<K>::Write(archive, key);
				Serializer<V>::Write(archive, value);
			}
		}
		static inline void Read(Input& archive, std::unordered_map<K, V>& map) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);
			map.reserve(num);

			for (size_t index = 0; index < num; ++index) {
				K key = DefaultReadValue<K>();
				V value = DefaultReadValue<V>();

				Serializer<K>::Read(archive, key);
				Serializer<V>::Read(archive, value);

				map.emplace(key, value);
			}
		}
	};
}

namespace Reflection {
	template<typename MapType, typename KeyType, typename ValueType>
	struct TMapTypeInfo : public ImplementedTypeInfo<MapType, MapTypeInfo> {
		using ImplementedTypeInfo<MapType, MapTypeInfo>::Cast;
		using MapTypeInfo::keys;
		using MapTypeInfo::values;

		TMapTypeInfo(std::u16string_view name, std::u16string_view description) : ImplementedTypeInfo<MapType, MapTypeInfo>(Reflect<MapType>::ID, name, desription) {
			keys = Reflect<KeyType>::Get();
			values = Reflect<ValueType>::Get();
		}

		static constexpr KeyType const& CastKey(void const* key) { return *static_cast<KeyType const*>(key); }
		static constexpr ValueType const& CastValue(void const* value) { return *static_cast<ValueType const*>(value); }

		virtual size_t GetCount(void const* instance) const override final { return Cast(instance).size(); }

		virtual void ForEachPair(void* instance, FunctionRef<bool(std::pair<void const*, void*>)> functor) const override final {
			for (auto& iter : Cast(instance)) {
				if (!functor(std::make_pair(&iter.first, &iter.second))) break;
			}
		}
		virtual void ForEachPair(void const* instance, FunctionRef<bool(std::pair<void const*, void const*>)> functor) const override final {
			for (auto const& iter : Cast(instance)) {
				if (!functor(std::make_pair(&iter.first, &iter.second))) break;
			}
		}

		virtual void* Find(void* instance, void const* key) const override final {
			auto const iter = Cast(instance).find(CastKey(key));
			if (iter != Cast(instance).end()) return &(*iter);
			else return nullptr;
		}
		virtual void const* Find(void const* instance, void const* key) const override final {
			auto const iter = Cast(instance).find(CastKey(key));
			if (iter != Cast(instance).end()) return &(*iter);
			else return nullptr;
		}

		virtual void Clear(void* instance) const override final { Cast(instance).clear(); }

		virtual bool RemoveEntry(void* instance, void const* key) const override final {
			size_t removedCount = Cast(instance).erase(CastKey(key));
			return removedCount > 0;
		}
		virtual bool InsertEntry(void* instance, void const* key, void const* value) const override final {
			if (value) return Cast(instance).insert(std::make_pair(CastKey(key), CastValue(value))).second;
			else return Cast(instance).insert(std::make_pair(CastKey(key), ValueType{})).second;
		}
	};
}

template<typename K, typename V>
struct Reflect<std::map<K, V>> {
	static ::Reflection::MapTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::map" } + Reflect<K>::ID + Reflect<V>::ID;
private:
	using ThisTypeInfo = ::Reflection::TMapTypeInfo<std::map<K, V>, K, V>;
	static ThisTypeInfo const info;
};

template<typename K, typename V>
typename Reflect<std::map<K, V>>::ThisTypeInfo const Reflect<std::map<K, V>>::info =
	Reflect<std::map<K, V>>::ThisTypeInfo{ u"std::map"sv, u"map"sv };

template<typename K, typename V>
struct Reflect<std::unordered_map<K, V>> {
	static ::Reflection::MapTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::unordered_map" } + Reflect<K>::ID + Reflect<V>::ID;
private:
	using ThisTypeInfo = ::Reflection::TMapTypeInfo<std::unordered_map<K, V>, K, V>;
	static ThisTypeInfo const info;
};

template<typename K, typename V>
typename Reflect<std::unordered_map<K, V>>::ThisTypeInfo const Reflect<std::unordered_map<K, V>>::info =
	Reflect<std::unordered_map<K, V>>::ThisTypeInfo{ u"std::unordered_map"sv, u"unordered_map"sv };
