#pragma once
#include <tuple>
#include "Engine/TupleUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TupleTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Tuple;

		/** The number of elements in the tuple */
		size_t Size = 0;

		TupleTypeInfo() = delete;
		TupleTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			size_t InSize
		);
		virtual ~TupleTypeInfo() = default;

		/** Get the type of the value at a specific index in the tuple */
		virtual TypeInfo const* GetValueType(size_t Index) const = 0;

		/** Get the value at a specific index in the tuple */
		virtual void* GetValue(void* Instance, size_t Index) const = 0;
		virtual void const* GetValue(void const* Instance, size_t Index) const = 0;
	};

	//============================================================
	// Templates

	/** Tuple element visitor which returns a type-erased pointer to the element */
	struct PointerVisitor {
		template<typename T>
		void* operator()(T& Element) const { return static_cast<void*>(&Element); }
		template<typename T>
		void const* operator()(T const& Element) const { return static_cast<void const*>(&Element); }
	};

	template<typename TupleType, typename ... ElementTypes>
	struct TTupleTypeInfo : public TupleTypeInfo {
		TTupleTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: TupleTypeInfo(
			TypeResolver<TupleType>::GetID(), GetCompilerDefinition<TupleType>(),
			InDescription, InFlags, InSerializer,
			std::tuple_size<TupleType>::value)
		{}

		STANDARD_TYPEINFO_METHODS(TupleType)

		virtual TypeInfo const* GetValueType(size_t Index) const final {
			return TupleUtility::VisitTypeAt<TypeInfo const*, TupleType, TypeResolver>(Index);
		}

		virtual void* GetValue(void* Instance, size_t Index) const final {
			return TupleUtility::VisitAt<void*>(Cast(Instance), Index, PointerVisitor{});
		}
		virtual void const* GetValue(void const* Instance, size_t Index) const final {
			return TupleUtility::VisitAt<void const*>(Cast(Instance), Index, PointerVisitor{});
		}
	};
}
