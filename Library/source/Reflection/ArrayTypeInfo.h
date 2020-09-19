#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include "Reflection/TypeResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct ArrayTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Array;

		/** Whether the number of elements in the array can be manipulated */
		bool isFixedSize = false;
		/** The type of the elements in the array */
		TypeInfo const* elementType = nullptr;

		ArrayTypeInfo() = delete;
		ArrayTypeInfo(Hash128 inID, CompilerDefinition inDef);
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
	struct TFixedArrayTypeInfo : public ArrayTypeInfo {
		TFixedArrayTypeInfo(std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer)
		: ArrayTypeInfo(TypeResolver<ArrayType>::GetID(), GetCompilerDefinition<ArrayType>())
		{
			isFixedSize = true;
			elementType = TypeResolver<ElementType>::Get();
		}

		STANDARD_TYPEINFO_METHODS(ArrayType)

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
	};

	/** Template that implements the ArrayTypeInfo interface for dynamic array types (std::vector, std::list, etc) */
	template<typename ArrayType, typename ElementType>
	struct TDynamicArrayTypeInfo : public ArrayTypeInfo {
		TDynamicArrayTypeInfo(std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer)
		: ArrayTypeInfo(TypeResolver<ArrayType>::GetID(), GetCompilerDefinition<ArrayType>())
		{
			isFixedSize = false;
			elementType = TypeResolver<ElementType>::Get();
		}

		STANDARD_TYPEINFO_METHODS(ArrayType)

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
	};
}
