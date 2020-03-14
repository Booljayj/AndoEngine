#include <array>
#include <cstddef>

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
using VariablePointerStorage = std::aligned_union<0, PointerTypes::StaticVariable, PointerTypes::MemberVariable>::type;
/** An untyped storage buffer large enough to contain any kind of function pointer */
using FunctionPointerStorage = std::aligned_union<0, PointerTypes::StaticFunction, PointerTypes::MemberFunction>::type;

/** Cast an aligned storage buffer to a specific type contained in the buffer */
template<typename T, typename... Types>
static T const& Cast(std::aligned_union_t<0, Types...> const& storage) { return std::launder(reinterpret_cast<T const*>(&storage)); }
template<typename T, typename... Types>
static T& Cast(std::aligned_union_t<0, Types...>& storage) { return std::launder(reinterpret_cast<T*>(&storage)); }
