#pragma once
#include <iterator>
#include "Reflection/StructTypeInfo.h"

template<typename TELEMENT, typename ... TARGS>
constexpr std::array<TELEMENT, sizeof...( TARGS )> CreateSortedArray( TARGS ... Args ) {
	std::array<TELEMENT, sizeof...( TARGS )> Result = { Args ... };
	//@todo std::sort is not constexpr until C++20. Fuck! Gotta find a good way to do this, it's essential.
	//std::sort( Result.begin(), Result.end(), []( auto const* A, auto const* B ) { return A->NameHash < B->NameHash; } );
	return Result;
}

template<typename TELEMENT, size_t SIZE>
constexpr std::basic_string_view<TELEMENT> MakeArrayView( const std::array<TELEMENT, SIZE>& Array ) {
	return std::basic_string_view<TELEMENT>{ Array.data(), Array.size() };
}

/** Begin the type reflection block */
#define REFLECTED_STRUCT_BEGIN( __STRUCT__ )\
namespace __STRUCT__ ## _REFLECTION {\
	using REFLECTED_TYPE = __STRUCT__;\
	__STRUCT__ const Default_REFLECTION{};

#define REFLECTED_STRUCT_END() }

#define REFLECT_STATIC_CONSTANT( __NAME__, __DESC__ )\
Reflection::TStaticConstantInfo<decltype( REFLECTED_TYPE::__NAME__ )> __NAME__{ #__NAME__, __DESC__, &REFLECTED_TYPE::__NAME__ }
#define REFLECT_MEMBER_CONSTANT( __NAME__, __DESC__ )\
Reflection::TMemberConstantInfo<REFLECTED_TYPE, decltype( REFLECTED_TYPE::__NAME__ )> __NAME__{ #__NAME__, __DESC__, &REFLECTED_TYPE::__NAME__ }

#define REFLECT_STATIC_VARIABLE( __NAME__, __DESC__ )\
Reflection::TStaticVariableInfo<decltype( REFLECTED_TYPE::__NAME__ )> __NAME__{ #__NAME__, __DESC__, &REFLECTED_TYPE::__NAME__ }
#define REFLECT_MEMBER_VARIABLE( __NAME__, __DESC__ )\
Reflection::TMemberVariableInfo<REFLECTED_TYPE, decltype( REFLECTED_TYPE::__NAME__ )> __NAME__{ #__NAME__, __DESC__, &REFLECTED_TYPE::__NAME__ }

#define _DEFINE_FIELDS( __SCOPE__, __TYPE__, ... )\
Reflection::__TYPE__ ## Info const* __SCOPE__ ## __TYPE__ ## Fields_REFLECTION[] = { __VA_ARGS__ };\
constexpr size_t __SCOPE__ ## __TYPE__ ## FieldsCount_REFLECTION = sizeof( __SCOPE__ ## __TYPE__ ## Fields_REFLECTION ) / sizeof( Reflection::__TYPE__ ## Info const* );

#define DEFINE_STATIC_CONSTANT_FIELDS( ... ) constexpr auto StaticConstantFields_REFLECTION = CreateSortedArray<Reflection::ConstantInfo const*>( __VA_ARGS__ )
#define DEFINE_MEMBER_CONSTANT_FIELDS( ... ) constexpr auto MemberConstantFields_REFLECTION = CreateSortedArray<Reflection::ConstantInfo const*>( __VA_ARGS__ )
#define DEFINE_STATIC_VARIABLE_FIELDS( ... ) constexpr auto StaticVariableFields_REFLECTION = CreateSortedArray<Reflection::VariableInfo const*>( __VA_ARGS__ )
#define DEFINE_MEMBER_VARIABLE_FIELDS( ... ) constexpr auto MemberVariableFields_REFLECTION = CreateSortedArray<Reflection::VariableInfo const*>( __VA_ARGS__ )

#define DEFINE_REFLECTION_MEMBERS( __STRUCT__, __DESC__ )\
Reflection::TStructTypeInfo<__STRUCT__> const __STRUCT__::_TypeInfo{\
	__DESC__,\
	Reflection::FTypeFlags::None, nullptr,\
	static_cast<void const*>( &__STRUCT__ ## _REFLECTION::Default_REFLECTION ),\
	Reflection::Fields{ MakeArrayView( __STRUCT__ ## _REFLECTION::StaticConstantFields_REFLECTION ), MakeArrayView( __STRUCT__ ## _REFLECTION::StaticVariableFields_REFLECTION ) },\
	Reflection::Fields{ MakeArrayView( __STRUCT__ ## _REFLECTION::MemberConstantFields_REFLECTION ), MakeArrayView( __STRUCT__ ## _REFLECTION::MemberVariableFields_REFLECTION ) }\
}
