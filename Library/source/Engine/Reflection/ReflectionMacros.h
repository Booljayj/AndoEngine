#pragma once

//============================================================
// Reflect macros

/** Declare a reflect implementation for a type */
#define REFLECT(QualifiedType, Classification)\
template<> struct ::Reflection::Reflect<QualifiedType> {\
	static ::Reflection::Classification ## TypeInfo const& Get();\
	static constexpr Hash128 ID = Hash128{ std::string_view{ STRINGIFY(QualifiedType) } };\
}

/** Define a reflect implementation for a type. A TypeInfo instance with the name InfoName must exist in the translation unit. */
#define DEFINE_REFLECT(QualifiedType, Classification, InfoName)\
::Reflection::Classification ## TypeInfo const& ::Reflection::Reflect<QualifiedType>::Get() { return InfoName; }

//============================================================
// Struct reflection macros

/** Declare members of a struct used for reflection. The second argument must be either the primary baseType class of this type or void */
#define REFLECT_STRUCT(StructType, StructBaseType)\
using ThisType = StructType;\
using BaseType = StructBaseType;\
static ::Reflection::TStructTypeInfo<ThisType> const info_ ## StructType;\
virtual ::Reflection::StructTypeInfo const& GetTypeInfo() const { return info_ ## StructType; }

/** Define members of a struct used for reflection */
#define DEFINE_REFLECT_STRUCT(Namespace, StructType)\
DEFINE_REFLECT(Namespace::StructType, Struct, Namespace::StructType::info_ ## StructType)\
::Reflection::TStructTypeInfo<Namespace::StructType> const Namespace::StructType::info_ ## StructType =\
	::Reflection::TStructTypeInfo<Namespace::StructType>{ std::string_view{ STRINGIFY(Namespace::StructType) } }\
	.Base<Namespace::StructType::BaseType>()

//============================================================
// Alias reflection macros

/** Declare members of an alias used for reflection */
#define REFLECT_ALIAS(AliasType)\
using ThisType = AliasType;\
static ::Reflection::TAliasTypeInfo<ThisType> const info_ ## AliasType

/** Define members of an alias used for reflection */
#define DEFINE_REFLECT_ALIAS(Namespace, AliasType)\
DEFINE_REFLECT(Namespace::AliasType, Alias, Namespace::AliasType::info_ ## AliasType)\
::Reflection::TAliasTypeInfo<Namespace::AliasType> const Namespace::AliasType::info_ ## AliasType =\
	::Reflection::TAliasTypeInfo<Namespace::AliasType>{ std::string_view{ STRINGIFY(Namespace::AliasType) } }
