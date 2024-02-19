#pragma once
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	/** TypeInfo for an array type */
	struct ArrayTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Array;

		/** Whether the number of elements in the array can be manipulated */
		bool isFixedSize = false;
		/** The type of the elements in the array */
		TypeInfo const* elements = nullptr;

		virtual ~ArrayTypeInfo() = default;

		/** Get the number of elements that are in the array */
		virtual size_t GetCount(void const* instance) const = 0;

		/** Get a vector of all the elements in the array */
		virtual void GetElements(void* instance, std::vector<void*>& outElements) const = 0;
		virtual void GetElements(void const* instance, std::vector<void const*>& outElements) const = 0;

		/** Resize the array to hold a specific number of elements */
		virtual void Resize(void* instance, size_t count) const = 0;

		/** Remove all elements in the container */
		virtual void ClearElements(void* instance) const = 0;
		/** Add a new elementType to the "end" of the array */
		virtual void AddElement(void* instance, void const* value) const = 0;

		/** Remove the pointed-at elementType */
		virtual void RemoveElement(void* instance, void const* element) const = 0;
		/** Insert a new elementType at the position of the pointed-at elementType, equal to the value. If value is nullptr, the new elementType is default-constructed */
		virtual void InsertElement(void* instance, void const* element, void const* value) const = 0;
	};

	//============================================================
	// Templates

	/** Template that implements the ArrayTypeInfo interface for fixed-size arrays (std::array) */
	template<typename ArrayType, typename ElementType, size_t Size>
	struct TFixedArrayTypeInfo : public ImplementedTypeInfo<ArrayType, ArrayTypeInfo> {
		using ImplementedTypeInfo<ArrayType, ArrayTypeInfo>::Cast;
		using ArrayTypeInfo::isFixedSize;
		using ArrayTypeInfo::elements;

		TFixedArrayTypeInfo(std::string_view inName) : ImplementedTypeInfo<ArrayType, ArrayTypeInfo>(Reflect<ArrayType>::ID, inName) {
			isFixedSize = true;
			elements = Reflect<ElementType>::Get();
		}

		virtual size_t GetCount(void const* instance) const final { return Size; }

		virtual void GetElements(void* instance, std::vector<void*>& outElements) const final {
			outElements.clear();
			for (ElementType& element : Cast(instance)) {
				outElements.push_back(&element);
			}
		}
		virtual void GetElements(void const* instance, std::vector<void const*>& outElements) const final {
			outElements.clear();
			for (ElementType const& element : Cast(instance)) {
				outElements.push_back(&element);
			}
		}

		virtual void Resize(void* instance, size_t count) const final {}
		virtual void ClearElements(void* instance) const final {}
		virtual void AddElement(void* instance, void const* value) const final {}
		virtual void RemoveElement(void* instance, void const* element) const final {}
		virtual void InsertElement(void* instance, void const* element, void const* value) const final {}

		TYPEINFO_BUILDER_METHODS(ArrayType)
	};

	/** Template that implements the ArrayTypeInfo interface for dynamic array types (std::vector, std::list, etc) */
	template<typename ArrayType, typename ElementType>
	struct TDynamicArrayTypeInfo : public ImplementedTypeInfo<ArrayType, ArrayTypeInfo> {
		using ImplementedTypeInfo<ArrayType, ArrayTypeInfo>::Cast;
		using ArrayTypeInfo::isFixedSize;
		using ArrayTypeInfo::elements;

		TDynamicArrayTypeInfo(std::string_view inName) : ImplementedTypeInfo<ArrayType, ArrayTypeInfo>(Reflect<ArrayType>::ID, inName) {
			isFixedSize = false;
			elements = &Reflect<ElementType>::Get();
		}

		static constexpr ElementType const& CastElement(void const* element) { return *static_cast<ElementType const*>(element); }

		virtual size_t GetCount(void const* instance) const final { return Cast(instance).size(); }

		virtual void GetElements(void* instance, std::vector<void*>& outElements) const final {
			outElements.clear();
			for (ElementType& element : Cast(instance)) {
				outElements.push_back(&element);
			}
		}
		virtual void GetElements(void const* instance, std::vector<void const*>& outElements) const final {
			outElements.clear();
			for (ElementType const& element : Cast(instance)) {
				outElements.push_back(&element);
			}
		}

		virtual void Resize(void* instance, size_t count) const final { Cast(instance).resize(count); }
		virtual void ClearElements(void* instance) const final { Cast(instance).clear(); }

		virtual void AddElement(void* instance, void const* value) const final {
			Cast(instance).push_back(CastElement(value));
		}
		virtual void RemoveElement(void* instance, void const* element) const final {
			const auto position = std::find_if(
				Cast(instance).begin(), Cast(instance).end(),
				[=](ElementType const& e) { return &e == element; }
			);
			Cast(instance).erase(position);
		}
		virtual void InsertElement(void* instance, void const* element, void const* value) const final {
			const auto position = std::find_if(
				Cast(instance).begin(), Cast(instance).end(),
				[=](ElementType const& e) { return &e == element; }
			);
			Cast(instance).insert(position, value ? CastElement(value) : ElementType{});
		}

		TYPEINFO_BUILDER_METHODS(ArrayType)
	};

	//============================================================
	// Standard fixed array reflection

	template<typename ElementType, size_t Size>
	struct Reflect<std::array<ElementType, Size>> {
		static ArrayTypeInfo const& Get() { return info; }
		static constexpr Hash128 ID = Hash128{ "std::array"sv } + Reflect<ElementType>::GetID() + Hash128{ static_cast<uint64_t>(Size), 0 };
	private:
		using ThisTypeInfo = TFixedArrayTypeInfo<std::array<ElementType, Size>, ElementType, Size>;
		static ThisTypeInfo const info;
	};
	template<typename ElementType, size_t Size>
	typename Reflect<std::array<ElementType, Size>>::ThisTypeInfo const Reflect<std::array<ElementType, Size>>::info =
		Reflect<std::array<ElementType, Size>>::ThisTypeInfo{ "std::array"sv }
		.Description("fixed array");

	//============================================================
	// Standard dynamic array reflection

	#define L_REFLECT_DYNAMICARRAY(ArrayTemplate, DescriptionString)\
	template<typename ElementType>\
	struct Reflect<ArrayTemplate<ElementType>> {\
		static ArrayTypeInfo const& Get() { return info; }\
		static constexpr Hash128 ID = Hash128{ #ArrayTemplate } + Reflect<ElementType>::ID;\
	private:\
		using ThisTypeInfo = TDynamicArrayTypeInfo<ArrayTemplate<ElementType>, ElementType>;\
		static ThisTypeInfo const info;\
	};\
	template<typename ElementType>\
	typename Reflect<ArrayTemplate<ElementType>>::ThisTypeInfo const Reflect<ArrayTemplate<ElementType>>::info =\
		Reflect<ArrayTemplate<ElementType>>::ThisTypeInfo{ #ArrayTemplate }\
		.Description(DescriptionString)

	L_REFLECT_DYNAMICARRAY(std::vector, "dynamic array"sv);
	L_REFLECT_DYNAMICARRAY(std::forward_list, "singly-linked list"sv);
	L_REFLECT_DYNAMICARRAY(std::list, "doubly-linked list"sv);
	L_REFLECT_DYNAMICARRAY(std::deque, "double-ended queue"sv);

	#undef L_REFLECT_DYNAMICARRAY
}
