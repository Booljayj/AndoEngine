#pragma once

/** Begin the type reflection block */
#define STRUCT_TYPE_BEGIN( __TYPE__ )\
Reflection::TStructTypeInfo<__TYPE__> const __TYPE__::__TypeInfo__{\
	[]( Reflection::StructTypeInfo* StructInfo ) {\
		using T = __TYPE__;\

#define MAKE_DEFAULT()\
StructInfo->Default = std::make_unique<char[]>( sizeof( T ) );\
new (StructInfo->Default.get()) T

#define MAKE_SERIALIZER()\
Serialization::StructSerializer* SerializerInstance = new Serialization::StructSerializer( StructInfo );\
SerializerInstance->Initialize();\
StructInfo->Serializer = std::unique_ptr<Serialization::ISerializer>{ SerializerInstance }

/** End the type reflection block */
#define STRUCT_TYPE_END( __TYPE__ )\
	}\
};\
Reflection::TypeInfo const* __TYPE__::GetTypeInfo() const { return &__TYPE__::__TypeInfo__; }

/** Add a static constant to the struct type info */
#define ADD_STATIC_CONSTANT( __VAR__, __DESC__ )\
StructInfo->StaticConstants.emplace_back(\
	new Reflection::TStaticConstantInfo<decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)
/** Add a member constant to the struct type info */
#define ADD_MEMBER_CONSTANT( __VAR__, __DESC__ )\
StructInfo->MemberConstants.emplace_back(\
	new Reflection::TMemberConstantInfo<T, decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)

/** Add a static variable to the struct type info */
#define ADD_STATIC_VARIABLE( __VAR__, __DESC__ )\
StructInfo->StaticVariables.emplace_back(\
	new Reflection::TStaticVariableInfo<decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)
/** Add a member variable to the struct type info */
#define ADD_MEMBER_VARIABLE( __VAR__, __DESC__ )\
StructInfo->MemberVariables.emplace_back(\
	new Reflection::TMemberVariableInfo<T, decltype(T::__VAR__)>( #__VAR__, __DESC__, &T::__VAR__ )\
)
