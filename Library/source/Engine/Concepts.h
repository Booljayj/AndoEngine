#pragma once
#include <concepts>
#include <type_traits>
#include <utility>

namespace Concepts {
	/** A type that is an enum */
	template<typename T>
	concept Enumeration = std::is_enum_v<T>;

	/** An integral or numeric type */
	template<typename T>
	concept Arithmetic = std::integral<T> || std::floating_point<T>;

	/** A type of character which can be used to create a string */
	template<typename T>
	concept Character = std::is_same_v<T, char> || std::is_same_v<T, wchar_t> || std::is_same_v<T, char8_t> || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

	/** A type which can be compared to nullptr, but is not literally nullptr */
	template<typename T>
	concept NullComparable =
		std::equality_comparable_with<std::nullptr_t, T> and
		std::copyable<T> and
		!std::is_null_pointer_v<T>;

	template<typename T, typename IndexType = size_t>
	concept Indexible = requires(T t, IndexType i) {
		t[i];
	};
}

/** Helper template struct that is created from a list of types */
template<typename... Types>
struct TTypeList {};
/** Helper variable that becomes a typelist created from a list of types */
template<typename... Types>
constexpr TTypeList<Types...> list;

/** Return the sizes of all types summed together */
template<typename... Types>
constexpr size_t SumTypeSizes() { return (sizeof(Types) + ...); }

namespace PointerTraits {
	namespace Internal {
		using StaticVariable = void*;
		using StaticFunction = void(*)();

		//DummyStruct does not exist. It is never defined. We declare it here so that we can use it to define
		//the size of pointers to various kinds of members on each compiler/platform. Because the type is never
		//defined, the compiler is forced to use a worst-case-scenario size, which is the largest.
		struct DummyStruct;
		using MemberVariable = void* DummyStruct::*;
		using MemberFunction = void(DummyStruct::*)();
	}

	/** The maximum size of any possible variable pointer */
	constexpr size_t VariablePointerSize = std::max(sizeof(PointerTraits::Internal::StaticVariable), sizeof(PointerTraits::Internal::MemberVariable));
	/** The maximum alignment of any possible variable pointer */
	constexpr size_t VariablePointerAlign = std::max(alignof(PointerTraits::Internal::StaticVariable), alignof(PointerTraits::Internal::MemberVariable));

	/** The maximum size of any possible function pointer */
	constexpr size_t FunctionPointerSize = std::max(sizeof(PointerTraits::Internal::StaticFunction), sizeof(PointerTraits::Internal::MemberFunction));
	/** The maximum alignment of any possible function pointer */
	constexpr size_t FunctionPointerAlign = std::max(alignof(PointerTraits::Internal::StaticFunction), alignof(PointerTraits::Internal::MemberFunction));
}

/** Cast an aligned storage buffer to a specific type contained in the buffer */
template<typename T, typename U>
inline T& CastUntypedStorage(U& storage) { return *std::launder(reinterpret_cast<T*>(&storage)); }
template<typename T, typename U>
inline T const& CastUntypedStorage(U const& storage) { return *std::launder(reinterpret_cast<T const*>(&storage)); }

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

/** Create a trait which can determine whether a class has a method with a given name and signature */
#define HAS_METHOD_TRAIT(MethodName, MethodSignature)\
template<typename, typename T> struct Has_ ## MethodName {};\
template<typename C, typename Ret, typename... Args> struct Has_ ## MethodName<C, Ret(Args...)> {\
private:\
template<typename T> static constexpr auto check(T*) -> typename std::is_same<decltype(std::declval<T>().MethodName(std::declval<Args>()...)), Ret>::type;\
template<typename> static constexpr std::false_type check(...); \
typedef decltype(check<C>(nullptr)) type;\
public:\
static constexpr bool Value = type::value;\
};\
template<class T> inline constexpr bool Has_ ## MethodName ## _V = Has_GetTypeInfo<T, MethodSignature>::Value
