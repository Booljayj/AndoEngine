#pragma once
#include "Engine/STL.h"
#include "Reflection/StructTypeInfo.h"

template<typename ElementType, typename... ArgTypes>
constexpr std::array<ElementType, sizeof...(ArgTypes)> CreateSortedArray(ArgTypes... Args) {
	std::array<ElementType, sizeof...(ArgTypes)> Result = {Args...};
	//@todo std::sort is not constexpr until C++20. Fuck! Gotta find a good way to do this, it's essential.
	//std::sort( Result.begin(), Result.end(), []( auto const* A, auto const* B ) { return A->NameHash < B->NameHash; } );
	return Result;
}

template<typename ElementType, size_t Size>
constexpr std::basic_string_view<ElementType> MakeArrayView(const std::array<ElementType, Size>& Array) {
	return std::basic_string_view<ElementType>{Array.data(), Array.size()};
}

/** Begin the type reflection block */
#define REFLECTED_STRUCT_BEGIN(StructType)\
namespace StructType ## _REFLECTION {\
	using ReflectedType = StructType;\
	StructType const Default_REFLECTION{};

#define REFLECTED_STRUCT_END() }

#define REFLECT_STATIC_CONSTANT(Name, Desc)\
Reflection::TStaticConstantInfo<decltype(ReflectedType::Name)> Name{#Name, Desc, &ReflectedType::Name}
#define REFLECT_MEMBER_CONSTANT(Name, Desc)\
Reflection::TMemberConstantInfo<ReflectedType, decltype(ReflectedType::Name)> Name{#Name, Desc, &ReflectedType::Name}

#define REFLECT_STATIC_VARIABLE(Name, Desc)\
Reflection::TStaticVariableInfo<decltype(ReflectedType::Name)> Name{#Name, Desc, &ReflectedType::Name}
#define REFLECT_MEMBER_VARIABLE(Name, Desc)\
Reflection::TMemberVariableInfo<ReflectedType, decltype(ReflectedType::Name)> Name{#Name, Desc, &ReflectedType::Name}

#define _DEFINE_FIELDS(ScopeName, TypeName, ...)\
Reflection::TypeName ## Info const* ScopeName ## TypeName ## Fields_REFLECTION[] = {__VA_ARGS__};\
constexpr size_t ScopeName ## TypeName ## FieldsCount_REFLECTION = sizeof(ScopeName ## TypeName ## Fields_REFLECTION) / sizeof(Reflection::TypeName ## Info const*);

#define DEFINE_STATIC_CONSTANT_FIELDS(...) constexpr auto StaticConstantFields_REFLECTION = CreateSortedArray<Reflection::ConstantInfo const*>(__VA_ARGS__)
#define DEFINE_MEMBER_CONSTANT_FIELDS(...) constexpr auto MemberConstantFields_REFLECTION = CreateSortedArray<Reflection::ConstantInfo const*>(__VA_ARGS__)
#define DEFINE_STATIC_VARIABLE_FIELDS(...) constexpr auto StaticVariableFields_REFLECTION = CreateSortedArray<Reflection::VariableInfo const*>(__VA_ARGS__)
#define DEFINE_MEMBER_VARIABLE_FIELDS(...) constexpr auto MemberVariableFields_REFLECTION = CreateSortedArray<Reflection::VariableInfo const*>(__VA_ARGS__)

#define DEFINE_REFLECTION_MEMBERS(StructType, Desc)\
Reflection::TStructTypeInfo<StructType> const StructType::typeInfo_{\
	Desc, Reflection::FTypeFlags::None, nullptr,\
	static_cast<void const*>( &StructType ## _REFLECTION::Default_REFLECTION ),\
	Reflection::Fields{ StructType ## _REFLECTION::StaticConstantFields_REFLECTION, StructType ## _REFLECTION::StaticVariableFields_REFLECTION },\
	Reflection::Fields{ StructType ## _REFLECTION::MemberConstantFields_REFLECTION, StructType ## _REFLECTION::MemberVariableFields_REFLECTION }\
}
