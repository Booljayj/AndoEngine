#pragma once

//============================================================
// Reflect macros

/** Define a reflect implementation for a type. A TypeInfo instance with the name InfoName must exist in the translation unit. */
#define DEFINE_REFLECT(QualifiedType, Classification, InfoName)\
::Reflection::Classification ## TypeInfo const& ::Reflect<QualifiedType>::Get() { return InfoName; }

//============================================================
// Struct reflection macros

/** Declare members of a struct used for reflection. The second argument must be either the primary baseType class of this type or void */
#define DECLARE_STRUCT_REFLECTION_MEMBERS(StructType, StructBaseType)\
using ThisType = StructType;\
using BaseType = StructBaseType;\
static ::Reflection::TStructTypeInfo<ThisType> const info_ ## StructType;\
virtual ::Reflection::StructTypeInfo const& GetTypeInfo() const { return info_ ## StructType; }

/** Define members of a struct used for reflection */
#define DEFINE_STRUCT_REFLECTION_MEMBERS(Namespace, StructType, Description)\
DEFINE_REFLECT(Namespace::StructType, Struct, Namespace::StructType::info_ ## StructType)\
::Reflection::TStructTypeInfo<Namespace::StructType> const Namespace::StructType::info_ ## StructType =\
	::Reflection::TStructTypeInfo<Namespace::StructType>{ STRINGIFY_U16(Namespace::StructType) ## sv, u ## Description ## sv }\
		.Base<Namespace::StructType::BaseType>()

//============================================================
// Alias reflection macros

/** Declare members of an alias used for reflection */
#define REFLECT_ALIAS(AliasType)\
using ThisType = AliasType;\
static ::Reflection::TAliasTypeInfo<ThisType> const info_ ## AliasType

/** Define members of an alias used for reflection */
#define DEFINE_REFLECT_ALIAS(Namespace, AliasType, Description)\
DEFINE_REFLECT(Namespace::AliasType, Alias, Namespace::AliasType::info_ ## AliasType)\
::Reflection::TAliasTypeInfo<Namespace::AliasType> const Namespace::AliasType::info_ ## AliasType =\
	::Reflection::TAliasTypeInfo<Namespace::AliasType>{ STRINGIFY_U16(Namespace::AliasType) ## sv, u ## Description ## sv }
