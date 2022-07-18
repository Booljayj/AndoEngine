#pragma once
#include "Engine/STL.h"

/** Helper template struct that is created from a list of types */
template<typename... Types>
struct TTypeList {};
/** Helper variable that becomes a typelist created from a list of types */
template<typename... Types>
constexpr TTypeList<Types...> list;

/** Return the sizes of all types summed together */
template<typename... Types>
constexpr size_t SumTypeSizes() { return (sizeof(Types) + ...); }

namespace PointerTypes {
	using StaticVariable = void*;
	using StaticFunction = void(*)();

	//DummyStruct does not exist. It is never defined. We declare it here so that we can use it to define
	//the size of pointers to various kinds of members on each compiler/platform. Because the type is never
	//defined, the compiler is forced to use the worst-case-scenario size, which is the largest.
	struct DummyStruct;
	using MemberVariable = void* DummyStruct::*;
	using MemberFunction = void(DummyStruct::*)();
}

/** An untyped storage buffer large enough to contain any kind of variable pointer */
using VariablePointerStorage = std::aligned_union_t<0, PointerTypes::StaticVariable, PointerTypes::MemberVariable>;
/** An untyped storage buffer large enough to contain any kind of function pointer */
using FunctionPointerStorage = std::aligned_union_t<0, PointerTypes::StaticFunction, PointerTypes::MemberFunction>;

/** Cast an aligned storage buffer to a specific type contained in the buffer */
template<typename T, typename U>
inline T& CastAlignedUnion(U& storage) { return *std::launder(reinterpret_cast<T*>(&storage)); }
template<typename T, typename U>
inline T const& CastAlignedUnion(U const& storage) { return *std::launder(reinterpret_cast<T const*>(&storage)); }

template<typename T, typename EqualTo = T>
struct HasOperatorEquals
{
private:
    template<typename U, typename V>
    static auto equal(U*) -> decltype(std::declval<U>() == std::declval<V>());
    template<typename, typename>
    static auto equal(...) -> std::false_type;

public:
	static constexpr bool Value = std::is_same<bool, decltype(equal<T, EqualTo>(0))>::value;
};

template<class T, class EqualTo = T>
inline constexpr bool HasOperatorEquals_V = HasOperatorEquals<T, EqualTo>::Value;
