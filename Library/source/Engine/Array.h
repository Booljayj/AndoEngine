#pragma once
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <queue>
#include <stack>
#include <vector>
#include "Engine/Core.h"
#include "Engine/Ranges.h"
#include "Engine/Reflection/TypeInfo.h"

//std::stack and std::queue are not supported for reflection or serialization. They do not provide a way to inspect elements without modifying the container.
//If reflection or serialization is needed, std::deque or std::vector will provide that with the same queue or stack behavior.

namespace Archive {
	namespace Concepts {
		//A range which can be written to using the specified element type and a method which adds elements to the back of the range.
		template<typename R, typename T>
		concept ReadWritableRange =
			ranges::sized_range<R> &&
			std::same_as<ranges::range_value_t<R>, T> &&
			Concepts::ReadWritable<T> &&
			requires(R r, T t) {
				r.push_back(t);
			};

		/** A class which contains a reserve method, typically a container. */
		template<typename R>
		concept Reservable = requires(R r, size_t s) {
			r.reserve(s);
		};

		/** A class which contains a resize method, typically a container. */
		template<typename R>
		concept Resizeable = requires(R r, size_t s) {
			r.resize(s);
		};
	}

	template<Concepts::ReadWritable T, size_t N>
	struct Serializer<std::array<T, N>> {
		static void Write(Output& archive, std::array<T, N> const& value) {
			for (T const& element : value) Serializer<T>::Write(archive, element);
		}
		static void Read(Input& archive, std::array<T, N>& value) {
			for (T const& element : value) Serializer<T>::Read(archive, element);
		}
	};

	template<typename T, Concepts::ReadWritableRange<T> R>
	struct DynamicArraySerializer {
		static void Write(Output& archive, R const& range) {
			Serializer<size_t>::Write(archive, ranges::size(range));
			for (T const& element : range) Serializer<T>::Write(archive, element);
		}

		static void Read(Input& archive, R& range) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);

			if constexpr (Concepts::Resizeable<R> && std::is_default_constructible_v<T>) {
				range.resize(num);
				for (T& element : range) Serializer<T>::Read(archive, element);
			} else {
				if constexpr (Concepts::Reservable<R>) range.reserve(num);
				
				for (size_t index = 0; index < num; ++index) {
					T element = DefaultReadValue<T>();
					Serializer<T>::Read(archive, element);
					range.push_back(element);
				}
			}
		}
	};

	template<Concepts::ReadWritable T, typename AllocatorType>
	struct Serializer<std::deque<T, AllocatorType>> : public DynamicArraySerializer<T, std::deque<T, AllocatorType>> {};
	template<Concepts::ReadWritable T, typename AllocatorType>
	struct Serializer<std::forward_list<T, AllocatorType>> : public DynamicArraySerializer<T, std::forward_list<T, AllocatorType>> {};
	template<Concepts::ReadWritable T, typename AllocatorType>
	struct Serializer<std::list<T, AllocatorType>> : public DynamicArraySerializer<T, std::list<T, AllocatorType>> {};
	template<Concepts::ReadWritable T, typename AllocatorType>
	struct Serializer<std::vector<T, AllocatorType>> : public DynamicArraySerializer<T, std::vector<T, AllocatorType>> {};
}

namespace Reflection {
	/** Template that implements the ArrayTypeInfo interface for fixed-size arrays (std::array) */
	template<typename ArrayType, typename ElementType, size_t Size>
	struct TFixedArrayTypeInfo : public ImplementedTypeInfo<ArrayType, ArrayTypeInfo> {
		using ImplementedTypeInfo<ArrayType, ArrayTypeInfo>::Cast;
		using ArrayTypeInfo::isFixedSize;
		using ArrayTypeInfo::elements;

		TFixedArrayTypeInfo(std::u16string_view name) : ImplementedTypeInfo<ArrayType, ArrayTypeInfo>(Reflect<ArrayType>::ID, name, {}) {
			isFixedSize = true;
			elements = Reflect<ElementType>::Get();
		}

		virtual void ForEachElement(void* instance, FunctionRef<bool(void*, size_t)> functor) const override final {
			size_t index = 0;
			for (ElementType& element : Cast(instance)) {
				functor(&element, index);
				++index;
			}
		}
		virtual void ForEachElement(void const* instance, FunctionRef<bool(void const*, size_t)> functor) const override final {
			size_t index = 0;
			for (ElementType const& element : Cast(instance)) {
				functor(&element, index);
				++index;
			}
		}

		virtual size_t GetCount(void const* instance) const override final { return Size; }

		virtual void* GetElement(void* instance, size_t index) const override final {
			return &(Cast(instance)[index]);
		}
		virtual void const* GetElement(void const* instance, size_t index) const override final {
			return &(Cast(instance)[index]);
		}

		virtual bool Resize(void* instance, size_t count) const override final { return false; }
		virtual bool ClearElements(void* instance) const override final { return false; }
		virtual bool AddElement(void* instance, void const* value) const override final { return false; }
		virtual bool RemoveElement(void* instance, void const* element) const override final { return false; }
		virtual bool InsertElement(void* instance, void const* element, void const* value) const override final { return false; }
	};

	/** Template that implements the ArrayTypeInfo interface for dynamic array types (std::vector, std::list, etc) */
	template<typename ArrayType, typename ElementType>
	struct TDynamicArrayTypeInfo : public ImplementedTypeInfo<ArrayType, ArrayTypeInfo> {
		using ImplementedTypeInfo<ArrayType, ArrayTypeInfo>::Cast;
		using ArrayTypeInfo::isFixedSize;
		using ArrayTypeInfo::elements;

		TDynamicArrayTypeInfo(std::u16string_view name) : ImplementedTypeInfo<ArrayType, ArrayTypeInfo>(Reflect<ArrayType>::ID, name) {
			isFixedSize = false;
			elements = &Reflect<ElementType>::Get();
		}

		static constexpr ElementType const& CastElement(void const* element) { return *static_cast<ElementType const*>(element); }

		virtual void ForEachElement(void* instance, FunctionRef<bool(void*, size_t)> functor) const override final {
			size_t index = 0;
			for (ElementType& element : Cast(instance)) {
				if (!functor(&element, index)) break;
				++index;
			}
		}
		virtual void ForEachElement(void const* instance, FunctionRef<bool(void const*, size_t)> functor) const override final {
			size_t index = 0;
			for (ElementType const& element : Cast(instance)) {
				if (!functor(&element, index)) break;
				++index;
			}
		}

		virtual size_t GetCount(void const* instance) const override final { return Cast(instance).size(); }
		virtual void* GetElement(void* instance, size_t index) const override final { return &(Cast(instance)[index]); }
		virtual void const* GetElement(void const* instance, size_t index) const override final { return &(Cast(instance)[index]); }

		virtual bool Resize(void* instance, size_t count) const override final { Cast(instance).resize(count); return true; }
		virtual bool ClearElements(void* instance) const override final { Cast(instance).clear(); return true; }
		virtual bool AddElement(void* instance, void const* value) const override final { Cast(instance).push_back(CastElement(value)); return true; }
		virtual bool RemoveElement(void* instance, void const* element) const override final {
			const auto position = std::find_if(
				Cast(instance).begin(), Cast(instance).end(),
				[=](ElementType const& e) { return &e == element; }
			);
			Cast(instance).erase(position);
			return true;
		}
		virtual bool InsertElement(void* instance, void const* element, void const* value) const override final {
			const auto position = std::find_if(
				Cast(instance).begin(), Cast(instance).end(),
				[=](ElementType const& e) { return &e == element; }
			);
			Cast(instance).insert(position, value ? CastElement(value) : ElementType{});
			return true;
		}
	};
}

template<typename ElementType, size_t Size>
struct Reflect<std::array<ElementType, Size>> {
	static ::Reflection::ArrayTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::array"sv } + Reflect<ElementType>::GetID() + Hash128{ static_cast<uint64_t>(Size), 0 };
private:
	using ThisTypeInfo = ::Reflection::TFixedArrayTypeInfo<std::array<ElementType, Size>, ElementType, Size>;
	static ThisTypeInfo const info;
};
template<typename ElementType, size_t Size>
typename Reflect<std::array<ElementType, Size>>::ThisTypeInfo const Reflect<std::array<ElementType, Size>>::info =
	Reflect<std::array<ElementType, Size>>::ThisTypeInfo{ "std::array"sv }
	.Description("fixed array"sv);

template<typename T>
struct Reflect<std::deque<T>> {
	static ::Reflection::ArrayTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::deque" } + Reflect<T>::ID;
private:
	using ThisTypeInfo = ::Reflection::TDynamicArrayTypeInfo<std::deque<T>, T>;
	static ThisTypeInfo const info;
};

template<typename T>
typename Reflect<std::deque<T>>::ThisTypeInfo const Reflect<std::deque<T>>::info =
	Reflect<std::deque<T>>::ThisTypeInfo{ "std::deque" }
	.Description("double-ended queue"sv);

template<typename T>
struct Reflect<std::forward_list<T>> {
	static ::Reflection::ArrayTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::forward_list" } + Reflect<T>::ID;
private:
	using ThisTypeInfo = ::Reflection::TDynamicArrayTypeInfo<std::forward_list<T>, T>;
	static ThisTypeInfo const info;
};
template<typename T>
struct Reflect<std::list<T>> {
	static ::Reflection::ArrayTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::list" } + Reflect<T>::ID;
private:
	using ThisTypeInfo = ::Reflection::TDynamicArrayTypeInfo<std::list<T>, T>;
	static ThisTypeInfo const info;
};

template<typename T>
typename Reflect<std::forward_list<T>>::ThisTypeInfo const Reflect<std::forward_list<T>>::info =
	Reflect<std::forward_list<T>>::ThisTypeInfo{ "std::forward_list" }
	.Description("forward list"sv);
template<typename T>
typename Reflect<std::list<T>>::ThisTypeInfo const Reflect<std::list<T>>::info =
	Reflect<std::list<T>>::ThisTypeInfo{ "std::list" }
	.Description("list"sv);

template<typename T>
struct Reflect<std::vector<T>> {
	static ::Reflection::ArrayTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::vector" } + Reflect<T>::ID;
private:
	using ThisTypeInfo = ::Reflection::TDynamicArrayTypeInfo<std::vector<T>, T>;
	static ThisTypeInfo const info;
};

template<typename T>
typename Reflect<std::vector<T>>::ThisTypeInfo const Reflect<std::vector<T>>::info =
	Reflect<std::vector<T>>::ThisTypeInfo{ "std::vector" }
	.Description("vector"sv);
