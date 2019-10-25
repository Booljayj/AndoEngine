#pragma once
#include <type_traits>

/** Test if the bitflag value is set to true in the mask */
#define TEST_BIT(Mask, Value) ((size_t)(Mask) & (size_t)(Value))
/** Set the bitflag to true in the mask */
#define SET_BIT(Mask, Value) ((size_t)(Mask) |= (size_t)(Value))
/** Set the bitflag to false in the mask */
#define CLEAR_BIT(Mask, Value) ((size_t)(Mask) &= (size_t)(~(Value)))

/**
 * Defines operators to work with a scoped enum that should act like a set of bitflags
 *     +A : returns A cast to its underlying integer type
 *     !A : returns true if A has no flags (use !! to check if A has any flags)
 *     ~A : returns A with flags flipped
 *     A|B : returns flags that are in A or B
 *     A&B : returns flags that are in both A and B
 *     A^B : returns flags that are different in A and B
 *     A+B : returns flags that are in A or B (alias for |)
 *     A-B : returns flags that are in A without any of the flags in B
 *     A*B : returns true if A contains any of the flags in B
 *     A/B : returns true if A contains all of the flags in B
 */
#define DEFINE_BITFLAG_OPERATORS(EnumType)\
constexpr inline std::underlying_type<EnumType>::type operator+(EnumType A) noexcept {\
	return static_cast<typename std::underlying_type<EnumType>::type>(A);\
}\
constexpr inline bool operator!(EnumType A) noexcept { return (+A) == 0; }\
constexpr inline EnumType operator~(EnumType A) noexcept { return static_cast<EnumType>(~(+A)); }\
constexpr inline EnumType operator|(EnumType A, EnumType B) noexcept { return static_cast<EnumType>((+A) | (+B)); }\
constexpr inline EnumType operator&(EnumType A, EnumType B) noexcept { return static_cast<EnumType>((+A) & (+B)); }\
constexpr inline EnumType operator^(EnumType A, EnumType B) noexcept { return static_cast<EnumType>((+A) ^ (+B)); }\
constexpr inline EnumType operator+(EnumType A, EnumType B) noexcept { return A|B; }\
constexpr inline EnumType operator-(EnumType A, EnumType B) noexcept { return static_cast<EnumType>((+A) & (~(+B)));; }\
constexpr inline EnumType& operator|=(EnumType& A, EnumType B) noexcept { A = A | B; return A; }\
constexpr inline EnumType& operator&=(EnumType& A, EnumType B) noexcept { A = A & B; return A; }\
constexpr inline EnumType& operator^=(EnumType& A, EnumType B) noexcept { A = A ^ B; return A; }\
constexpr inline EnumType& operator+=(EnumType& A, EnumType B) noexcept { A = A + B; return A; }\
constexpr inline EnumType& operator-=(EnumType& A, EnumType B) noexcept { A = A - B; return A; }\
[[deprecated("Use operator* instead")]]\
constexpr inline bool operator<(EnumType A, EnumType B) noexcept { return ((+A) & (+B)) != 0; }\
constexpr inline bool operator*(EnumType A, EnumType B) noexcept { return ((+A) & (+B)) != 0; }\
constexpr inline bool operator/(EnumType A, EnumType B) noexcept { return ((+A) & (+B)) == (+B); }\
