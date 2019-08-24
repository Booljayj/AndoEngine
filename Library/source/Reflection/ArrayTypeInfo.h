#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include "Reflection/TypeResolver.h"
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct ArrayTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Array;

		/** Whether the number of elements in the array can be manipulated */
		bool IsFixedSize = false;
		/** The type of the elements in the array */
		TypeInfo const* ElementTypeInfo = nullptr;

		ArrayTypeInfo() = delete;
		ArrayTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			bool InIsFixedSize, TypeInfo const* InElementTypeInfo
		);
		virtual ~ArrayTypeInfo() = default;

		/** Get the number of elements that are in the array */
		virtual size_t GetCount(void const* Instance) const = 0;

		/** Get a vector of all the elements in the array */
		virtual void GetElements(void* Instance, std::vector<void*>& OutElements) const = 0;
		virtual void GetElements(void const* Instance, std::vector<void const*>& OutElements) const = 0;

		/** Resize the array to hold a specific number of elements */
		virtual void Resize(void* Instance, size_t Count) const = 0;

		/** Remove all elements in the container */
		virtual void ClearElements(void* Instance) const = 0;
		/** Add a new element to the "end" of the array */
		virtual void AddElement(void* Instance, void const* Value) const = 0;

		/** Remove the pointed-at element */
		virtual void RemoveElement(void* Instance, void const* Element) const = 0;
		/** Insert a new element at the position of the pointed-at element, equal to the value. If Value is nullptr, the new element is default-constructed */
		virtual void InsertElement(void* Instance, void const* Element, void const* Value) const = 0;
	};

	//============================================================
	// Templates

	/** Template that implements the ArrayTypeInfo interface for fixed-size arrays (std::array) */
	template<typename ElementType, size_t Size, typename ArrayType>
	struct TFixedArrayTypeInfo : public ArrayTypeInfo {
		TFixedArrayTypeInfo(std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: ArrayTypeInfo(
			TypeResolver<ArrayType>::GetID(), GetCompilerDefinition<ArrayType>(),
			InDescription, InFlags, InSerializer,
			true, TypeResolver<ElementType>::Get())
		{}

		STANDARD_TYPEINFO_METHODS(ArrayType)

		virtual size_t GetCount(void const* Instance) const final { return Size; }

		virtual void GetElements(void* Instance, std::vector<void*>& OutElements) const final {
			OutElements.clear();
			for (ElementType& ArrayElement : Cast(Instance)) {
				OutElements.push_back(&ArrayElement);
			}
		}
		virtual void GetElements(void const* Instance, std::vector<void const*>& OutElements) const final {
			OutElements.clear();
			for (ElementType const& ArrayElement : Cast(Instance)) {
				OutElements.push_back(&ArrayElement);
			}
		}

		virtual void Resize(void* Instance, size_t Count) const final {}
		virtual void ClearElements(void* Instance) const final {}
		virtual void AddElement(void* Instance, void const* Value) const final {}
		virtual void RemoveElement(void* Instance, void const* Element) const final {}
		virtual void InsertElement(void* Instance, void const* Element, void const* Value) const final {}
	};

	/** Template that implements the ArrayTypeInfo interface for dynamic array types (std::vector, std::list, etc) */
	template<typename ElementType, typename ArrayType>
	struct TDynamicArrayTypeInfo : public ArrayTypeInfo {
		TDynamicArrayTypeInfo(std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: ArrayTypeInfo(
			TypeResolver<ArrayType>::GetID(), GetCompilerDefinition<ArrayType>(),
			InDescription, InFlags, InSerializer,
			false, TypeResolver<ElementType>::Get())
		{}

		STANDARD_TYPEINFO_METHODS(ArrayType)

		static constexpr ElementType const& CastElement(void const* Element) { return *static_cast<ElementType const*>(Element); }

		virtual size_t GetCount(void const* Instance) const final { return Cast(Instance).size(); }

		virtual void GetElements(void* Instance, std::vector<void*>& OutElements) const final {
			OutElements.clear();
			for (ElementType& ArrayElement : Cast(Instance)) {
				OutElements.push_back(&ArrayElement);
			}
		}
		virtual void GetElements(void const* Instance, std::vector<void const*>& OutElements) const final {
			OutElements.clear();
			for (ElementType const& ArrayElement : Cast(Instance)) {
				OutElements.push_back(&ArrayElement);
			}
		}

		virtual void Resize(void* Instance, size_t Count) const final { Cast(Instance).resize(Count); }
		virtual void ClearElements(void* Instance) const final { Cast(Instance).clear(); }

		virtual void AddElement(void* Instance, void const* Value) const final {
			Cast(Instance).push_back(CastElement(Value));
		}
		virtual void RemoveElement(void* Instance, void const* Element) const final {
			const auto Position = std::find_if(
				Cast(Instance).begin(), Cast(Instance).end(),
				[=](ElementType const& E) { return &E == Element; }
			);
			Cast(Instance).erase(Position);
		}
		virtual void InsertElement(void* Instance, void const* Element, void const* Value) const final {
			const auto Position = std::find_if(
				Cast(Instance).begin(), Cast(Instance).end(),
				[=](ElementType const& E) { return &E == Element; }
			);
			Cast(Instance).insert(Position, Value ? CastElement(Value) : ElementType{});
		}
	};
}
