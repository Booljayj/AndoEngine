#pragma once
#include <tuple>
#include "Engine/TupleUtility.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TupleTypeInfo : public TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Tuple;

		/** The number of elements in the tuple */
		size_t size = 0;

		TupleTypeInfo() = delete;
		TupleTypeInfo(
			Hash128 inID, CompilerDefinition inDef,
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer,
			size_t inSize
		);
		virtual ~TupleTypeInfo() = default;

		/** Get the type of the value at a specific index in the tuple */
		virtual TypeInfo const* GetValueType(size_t index) const = 0;

		/** Get the value at a specific index in the tuple */
		virtual void* GetValue(void* instance, size_t index) const = 0;
		virtual void const* GetValue(void const* instance, size_t index) const = 0;
	};

	//============================================================
	// Templates

	/** Tuple element visitor which returns a type-erased pointer to the element */
	struct PointerVisitor {
		template<typename T>
		void* operator()(T& element) const { return static_cast<void*>(&element); }
		template<typename T>
		void const* operator()(T const& element) const { return static_cast<void const*>(&element); }
	};

	template<typename TupleType, typename... ElementTypes>
	struct TTupleTypeInfo : public TupleTypeInfo {
		TTupleTypeInfo(
			std::string_view inDescription, FTypeFlags inFlags, Serialization::ISerializer* inSerializer)
		: TupleTypeInfo(
			TypeResolver<TupleType>::GetID(), GetCompilerDefinition<TupleType>(),
			inDescription, inFlags, inSerializer,
			std::tuple_size<TupleType>::value)
		{}

		STANDARD_TYPEINFO_METHODS(TupleType)

		virtual TypeInfo const* GetValueType(size_t index) const final {
			return TupleUtility::VisitTypeAt<TypeInfo const*, TupleType, TypeResolver>(index);
		}

		virtual void* GetValue(void* instance, size_t index) const final {
			return TupleUtility::VisitAt<void*>(Cast(instance), index, PointerVisitor{});
		}
		virtual void const* GetValue(void const* instance, size_t index) const final {
			return TupleUtility::VisitAt<void const*>(Cast(instance), index, PointerVisitor{});
		}
	};
}
