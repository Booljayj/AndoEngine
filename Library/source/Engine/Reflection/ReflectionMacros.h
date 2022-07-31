#pragma once
#include "Engine/Reflection/StructTypeInfo.h"

//============================================================
// Reflect macros

/** Declare a reflect implementation for a struct */
#define DECLARE_REFLECT(Type, TypeInfoType)\
template<> struct Reflect<Type> {\
	static ::Reflection::TypeInfoType ## TypeInfo const* Get();\
	static constexpr Hash128 ID = Hash128{ std::string_view{ STRINGIFY(Type) } };\
}

/** Define a reflect implementation for a type. A TypeInfo instance with the name InfoName must exist in the translation unit. */
#define DEFINE_REFLECT(Type, TypeInfoType, InfoName) ::Reflection::TypeInfoType ## TypeInfo const* Reflect<Type>::Get() { return &InfoName; }

//============================================================
// StructTypeInfo creation macros

/** Declare a StructTypeInfo variable for the given struct */
#define DECLARE_STRUCTTYPEINFO(Type, Name) ::Reflection::TStructTypeInfo<Type> const Name;

/** Define a StructTypeInfo variable for the given struct */
#define DEFINE_STRUCTTYPEINFO(Type, Name) ::Reflection::TStructTypeInfo<Type> const Name = ::Reflection::TStructTypeInfo<Type>{ std::string_view{ #Type } }

//============================================================
// Struct reflection macros

/** Declare members of a struct used for reflection. The second argument must be either the primary baseType class of this type or void */
#define DECLARE_STRUCT_REFLECTION_MEMBERS(StructType_, BaseType_)\
using BaseType = BaseType_;\
private:\
static DECLARE_STRUCTTYPEINFO(StructType_, info_ ## StructType_)\
public:\
virtual ::Reflection::StructTypeInfo const& GetTypeInfo() const { return info_ ## StructType_; }\
static ::Reflection::ImplementedTypeInfo<StructType_, ::Reflection::StructTypeInfo> const& TypeInfo() { return info_ ## StructType_; }

/** Define members of a struct used for reflection */
#define DEFINE_STRUCT_REFLECTION_MEMBERS(Namespace, StructType_)\
DEFINE_REFLECT(Namespace::StructType_, Struct, Namespace::StructType_::TypeInfo())\
DEFINE_STRUCTTYPEINFO(Namespace::StructType_, Namespace::StructType_::info_ ## StructType_).BaseType<Namespace::StructType_::BaseType>()

/** Create a VariableInfo instance for a member variable */
#define REFLECT_MVAR(StructType, MemberName, Description) ::Reflection::VariableInfo{ &StructType::MemberName, #MemberName, Description, ::Reflection::FVariableFlags::None }

//============================================================
// AliasTypeInfo creation macros

/** Declare an AliasTypeInfo variable for the given struct */
#define DECLARE_ALIASTYPEINFO(Type, Name) ::Reflection::TAliasTypeInfo<Type> const Name;

/** Define an AliasTypeInfo variable for the given struct */
#define DEFINE_ALIASTYPEINFO(Type, Name) ::Reflection::TAliasTypeInfo<Type> const Name = ::Reflection::TAliasTypeInfo<Type>{ std::string_view{ #Type } }

//============================================================
// Alias reflection macros

/** Declare members of an alias used for reflection */
#define DECLARE_ALIAS_REFLECTION_MEMBERS(AliasType_)\
private:\
static DECLARE_ALIASTYPEINFO(AliasType_, info_ ## AliasType_)\
public:\
static ::Reflection::ImplementedTypeInfo<AliasType_, ::Reflection::AliasTypeInfo> const& TypeInfo() { return info_ ## AliasType_; }

/** Define members of an alias used for reflection */
#define DEFINE_ALIAS_REFLECTION_MEMBERS(Namespace, AliasType_)\
DEFINE_REFLECT(Namespace::AliasType_, Alias, Namespace::AliasType_::TypeInfo())\
DEFINE_ALIASTYPEINFO(Namespace::AliasType_, Namespace::AliasType_::info_ ## AliasType_)
