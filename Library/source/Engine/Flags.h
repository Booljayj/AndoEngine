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

template<typename EnumType_>
struct TFlags {
public:
	using EnumType = EnumType_;
	using UnderlyingType = typename std::underlying_type<EnumType>::type;

	constexpr inline TFlags() : Flags(0) {}
	constexpr inline TFlags(const TFlags& Other) : Flags(Other.Flags) {}
	constexpr inline TFlags(UnderlyingType InFlags) : Flags(InFlags) {}
	constexpr inline TFlags(EnumType InFlag) : Flags(1 << (UnderlyingType)InFlag) {}

	constexpr inline bool operator==(TFlags Other) const noexcept { return Flags == Other.Flags; }
	constexpr inline bool operator!=(TFlags Other) const noexcept { return Flags != Other.Flags; }

	/** Get the underlying integer value for this set of flags */
	constexpr inline UnderlyingType operator+() noexcept { return Flags; }

	/** Add a value to the set of flags */
	constexpr inline TFlags operator+(EnumType Value) const { return Flags | (1 << (UnderlyingType)Value); }
	constexpr inline TFlags& operator+=(EnumType Value) { *this = *this + Value; return *this; }
	/** Remove a value from the set of flags */
	constexpr inline TFlags operator-(EnumType Value) const { return Flags & ~(1 << (UnderlyingType)Value); }
	constexpr inline TFlags& operator-=(EnumType Value) { *this = *this - Value; return *this; }

	/** Returns the set of flags in this or the other */
	constexpr inline TFlags Union(TFlags Other) const noexcept { return Flags | Other.Flags; }
	/** Returns the set of flags in this but without the flags in other */
	constexpr inline TFlags Subtraction(TFlags Other) const noexcept { return Flags & ~Other.Flags; }
	/** Returns the set of common flags in both this and the other */
	constexpr inline TFlags Intersection(TFlags Other) const noexcept { return Flags & Other.Flags; }
	/** Returns the set of flags that are different in this and the other */
	constexpr inline TFlags Difference(TFlags Other) const noexcept { return Flags ^ Other.Flags; }

	/** True if this set of flags contains the value */
	constexpr inline bool Has(EnumType Value) const noexcept { return (Flags & (1 << (UnderlyingType)Value)) != 0; }
	/** True if this set of flags contains no values */
	constexpr inline bool IsEmpty() const noexcept { return !!Flags; }

	/** True if this set of flags is a strict subset of another set of flags */
	constexpr inline bool IsSubsetOf(TFlags Other) const noexcept { return Intersection(Difference(Other)).IsEmpty(); }

private:
	UnderlyingType Flags;
};
