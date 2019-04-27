#pragma once
#include <type_traits>

/** Test if the bitflag value is set to true in the mask */
#define TEST_BIT( Mask, Bit ) ((size_t)(Mask) & (size_t)(Bit))
/** Set the bitflag to true in the mask */
#define SET_BIT( Mask, Bit ) ((size_t)(Mask) |= (size_t)(Bit))
/** Set the bitflag to false in the mask */
#define CLEAR_BIT( Mask, Bit ) ((size_t)(Mask) &= (size_t)(~(Bit)))

/** Defines operators to work with a scoped enum that should act like a set of bitflags */
#define DEFINE_BITFLAG_OPERATORS( __ENUM__ )\
constexpr inline __ENUM__ operator|( __ENUM__ A, __ENUM__ B ) {\
	return static_cast<__ENUM__>( static_cast<std::underlying_type<__ENUM__>::type>(A) | static_cast<std::underlying_type<__ENUM__>::type>(B) );\
}\
constexpr inline __ENUM__ operator&( __ENUM__ A, __ENUM__ B ) {\
	return static_cast<__ENUM__>( static_cast<std::underlying_type<__ENUM__>::type>(A) & static_cast<std::underlying_type<__ENUM__>::type>(B) );\
}\
constexpr inline __ENUM__ operator^( __ENUM__ A, __ENUM__ B ) {\
	return static_cast<__ENUM__>( static_cast<std::underlying_type<__ENUM__>::type>(A) ^ static_cast<std::underlying_type<__ENUM__>::type>(B) );\
}\
constexpr inline __ENUM__& operator|=( __ENUM__& A, __ENUM__ B ) { A = A | B; return A; }\
constexpr inline __ENUM__& operator&=( __ENUM__& A, __ENUM__ B ) { A = A & B; return A; }\
constexpr inline __ENUM__& operator^=( __ENUM__& A, __ENUM__ B ) { A = A ^ B; return A; }\
constexpr inline bool operator<( __ENUM__ A, __ENUM__ B ) {\
	return bool( static_cast<std::underlying_type<__ENUM__>::type>(A) & static_cast<std::underlying_type<__ENUM__>::type>(B) );\
}\
