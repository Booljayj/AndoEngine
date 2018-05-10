#pragma once

/** Implement the master reflection functions for the type */
#define DEFINE_REFLECTION( __TYPE__, __TYPE_NAME__ )\
Reflection::TypeInfo* __TYPE__::StaticGetTypeInfo() { return &Reflection::TypeInfo__ ## __TYPE_NAME__; }\
Reflection::TypeInfo* __TYPE__::GetTypeInfo() const { return __TYPE__::StaticGetTypeInfo(); }

/** Begin the type reflection block */
#define TYPE_REFLECT_BEGIN( __TYPE__, __TYPE_NAME__ )\
StructTypeInfo TypeInfo__ ## __TYPE_NAME__{\
	[]( TypeInfo* Info ) {\
		using T = __TYPE__;\

/** Begin the block that defines struct type info */
#define STRUCT_TYPE()\
if( auto* StructInfo = Info->As<Reflection::StructTypeInfo>() )

/** End the type reflection block */
#define TYPE_REFLECT_END( __TYPE__, __TYPE_NAME_STR__ )\
	},\
	__TYPE_NAME_STR__, sizeof( __TYPE__ ),\
};\

/** Add a static constant to the struct type info */
#define ADD_STATIC_CONSTANT( __VAR__, __DESC__ )\
StructInfo->StaticConstants.emplace_back(\
	new TStaticConstantInfo<decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)
/** Add a member constant to the struct type info */
#define ADD_MEMBER_CONSTANT( __VAR__, __DESC__ )\
StructInfo->MemberConstants.emplace_back(\
	new TMemberConstantInfo<T, decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)

/** Add a static variable to the struct type info */
#define ADD_STATIC_VARIABLE( __VAR__, __DESC__ )\
StructInfo->StaticVariables.emplace_back(\
	new TStaticVariableInfo<decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)
/** Add a member variable to the struct type info */
#define ADD_MEMBER_VARIABLE( __VAR__, __DESC__ )\
StructInfo->MemberVariables.emplace_back(\
	new TMemberVariableInfo<T, decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)
