#pragma once
#include <array>
#include <string_view>
#include <vector>
#include <type_traits>
#include "Engine/Hash.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/Components/ConstantInfo.h"
#include "Reflection/Components/VariableInfo.h"

/** Define members of the struct used for reflection. The second argument must be either the primary base class of this type or void */
#define REFLECTION_MEMBERS(_StructType_, _BaseType_)\
using BaseType = _BaseType_;\
static Reflection::TStructTypeInfo<_StructType_> const _TypeInfo

/** Expose a struct to the reflection system. Must be expanded after the type is declared and includes the REFELCTION_MEMBERS macro */
#define REFLECT(_StructType_)\
namespace Reflection {\
	namespace Internal {\
		template<> struct TypeResolver_Implementation<_StructType_> {\
			static TypeInfo const* Get() { return &_StructType_::_TypeInfo; }\
			static constexpr Hash128 GetID() { return Hash128{ #_StructType_ }; }\
		};\
	}\
}

namespace Reflection {
	/** A view type specialized for field components */
	template<typename T>
	struct FieldView {
	private:
		std::basic_string_view<T const*> InternalView;

	public:
		constexpr FieldView() = default;

		template<size_t SIZE>
		constexpr FieldView(std::array<T const*, SIZE> const& Array)
		: InternalView(Array.data(), Array.size())
		{}

		inline T const* operator[](size_t Index) const { return InternalView[Index]; }
		inline typename decltype(InternalView)::const_iterator begin() const { return InternalView.begin(); }
		inline typename decltype(InternalView)::const_iterator end() const { return InternalView.end(); }

		inline size_t Size() const { return InternalView.size(); }

		T const* Find(Hash32 NameHash) const {
			//@todo If we have a way to ensure that the field array is sorted, we can use a binary search here for better speed.
			const auto Iter = std::find_if(InternalView.begin(), InternalView.end(), [=](T const* Info) { return Info->NameHash == NameHash; });
			if (Iter != InternalView.end()) return *Iter;
			else return nullptr;
		}
	};

	/** Views into various field types that the struct defines */
	struct Fields {
		FieldView<ConstantInfo> Constants;
		FieldView<VariableInfo> Variables;
	};

	/** Info for a struct type, which contains various fields and supports inheritance */
	struct StructTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Struct;

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo const* BaseTypeInfo = nullptr;
		/** A default-constructed instance of this struct type, used to find default values for variables */
		void const* Default = nullptr;
		/** The static fields that this type defines */
		Fields Static;
		/** The member fields that this type defines */
		Fields Member;

		StructTypeInfo() = delete;
		StructTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			StructTypeInfo const* InBaseTypeInfo, void const* InDefault,
			Fields InStatic, Fields InMember
		);
		virtual ~StructTypeInfo() = default;

		/** Returns true if the chain of base types includes the provided type */
		bool DerivesFrom(TypeInfo const* OtherBaseTypeInfo) const { return true; }
	};

	//============================================================
	// Templates

	template<typename StructType>
	struct TStructTypeInfo : public StructTypeInfo {
		using BaseType = typename StructType::BaseType;
		static_assert(
			std::is_base_of<BaseType, StructType>::value || std::is_void<BaseType>::value,
			"invalid type inheritance for StructTypeInfo, T::BaseType is not void or an actual base of T"
		);

		TStructTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			void const* InDefault,
			Fields InStatic, Fields InMember)
		: StructTypeInfo(
			TypeResolver<StructType>::GetID(), GetCompilerDefinition<StructType>(),
			InDescription, InFlags, InSerializer,
			Reflection::Cast<StructTypeInfo>(TypeResolver<BaseType>::Get()), InDefault,
			InStatic, InMember)
		{}

		STANDARD_TYPEINFO_METHODS(StructType)
	};
}
