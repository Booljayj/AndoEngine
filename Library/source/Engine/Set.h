#pragma once
#include <set>
#include <unordered_set>
#include "Engine/Core.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Archive {
	template<Concepts::ReadWritable T>
	struct Serializer<std::set<T>> {
		static inline void Write(Output& archive, std::set<T> const& set) {
			Serializer<size_t>::Write(archive, set.size());
			for (const T& element : set) Serializer<T>::Write(archive, element);
		}
		static inline void Read(Input& archive, std::set<T>& set) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);

			for (size_t index = 0; index < num; ++index) {
				T element = DefaultReadValue<T>();
				Serializer<T>::Read(archive, element);
				set.emplace(element);
			}
		}
	};

	template<Concepts::ReadWritable T>
	struct Serializer<std::unordered_set<T>> {
		static inline void Write(Output& archive, std::unordered_set<T> const& set) {
			Serializer<size_t>::Write(archive, set.size());
			for (const T& element : set) Serializer<T>::Write(archive, element);
		}
		static inline void Read(Input& archive, std::unordered_set<T>& set) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);
			set.reserve(num);

			for (size_t index = 0; index < num; ++index) {
				T element = DefaultReadValue<T>();
				Serializer<T>::Read(archive, element);
				set.emplace(element);
			}
		}
	};
}

namespace Reflection {
	template<typename SetType, typename ValueType>
	struct TSetTypeInfo : public ImplementedTypeInfo<SetType, SetTypeInfo> {
		using ImplementedTypeInfo<SetType, SetTypeInfo>::Cast;
		using SetTypeInfo::values;

		TSetTypeInfo(std::u16string_view name, std::u16string_view description)
			: ImplementedTypeInfo<SetType, SetTypeInfo>(Reflect<SetType>::ID, name, desription)
			, values(Reflect<ValueType>::Get())
		{}

		static constexpr ValueType const& CastValue(void const* value) { return *static_cast<ValueType const*>(value); }

		virtual size_t GetCount(void const* instance) const final { return Cast(instance).size(); }

		virtual void ForEachElement(void const* instance, FunctionRef<bool(void const*)> functor) const override final {
			for (ValueType const& value : Cast(instance)) {
				if (!functor(&value)) break;
			}
		}

		virtual bool Contains(void const* instance, void const* value) const final {
			return Cast(instance).find(CastValue(value)) != Cast(instance).cend();
		}

		virtual void Clear(void* instance) const final { Cast(instance).clear(); }
		virtual bool Add(void* instance, void const* value) const final { return Cast(instance).insert(CastValue(value)).second; }
		virtual bool Remove(void* instance, void const* value) const final { return Cast(instance).erase(CastValue(value)) > 0; }
	};
}

template<typename T>
struct Reflect<std::set<T>> {
	static ::Reflection::SetTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::set" } + Reflect<T>::ID;
private:
	using ThisTypeInfo = ::Reflection::TSetTypeInfo<std::set<T>, T>;
	static ThisTypeInfo const info;
};

template<typename T>
typename Reflect<std::set<T>>::ThisTypeInfo const Reflect<std::set<T>>::info =
	Reflect<std::set<T>>::ThisTypeInfo{ u"std::set"sv, u"set"sv };

template<typename T>
struct Reflect<std::unordered_set<T>> {
	static ::Reflection::SetTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::unordered_set" } + Reflect<T>::ID;
private:
	using ThisTypeInfo = ::Reflection::TSetTypeInfo<std::unordered_set<T>, T>;
	static ThisTypeInfo const info;
};

template<typename T>
typename Reflect<std::unordered_set<T>>::ThisTypeInfo const Reflect<std::unordered_set<T>>::info =
	Reflect<std::unordered_set<T>>::ThisTypeInfo{ u"std::unordered_set"sv, u"unordered_set"sv };
