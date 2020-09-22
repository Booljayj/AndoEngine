#pragma once
#include "Engine/STL.h"

/** Test if the bitflag value is set to true in the mask */
#define TEST_BIT(Mask, Flag) ((size_t)(Mask) & (size_t)(Flag))
/** Set the bitflag to true in the mask */
#define SET_BIT(Mask, Flag) ((size_t)(Mask) |= (size_t)(Flag))
/** Set the bitflag to false in the mask */
#define CLEAR_BIT(Mask, Flag) ((size_t)(Mask) &= (size_t)(~(Flag)))

template<typename EnumType_>
struct TFlags {
public:
	using EnumType = EnumType_;
	using UnderlyingType = typename std::underlying_type<EnumType>::type;

	static const TFlags None;

	constexpr inline TFlags() : flags(0) {}
	constexpr inline TFlags(const TFlags& other) : flags(other.flags) {}
	constexpr inline TFlags(UnderlyingType inFlags) : flags(inFlags) {}
	constexpr inline TFlags(EnumType inFlag) : flags(1 << (UnderlyingType)inFlag) {}

	template<typename... FlagTypes>
	static constexpr inline TFlags Make(FlagTypes... inFlags) {
		static_assert(std::conjunction_v<std::is_same<FlagTypes, EnumType>...>, "Invalid types for creating flags");
		const UnderlyingType flags = (... | (1 << (UnderlyingType)inFlags));
		return TFlags{ flags };
	}

	constexpr inline bool operator==(TFlags other) const noexcept { return flags == other.flags; }
	constexpr inline bool operator!=(TFlags other) const noexcept { return flags != other.flags; }

	/** Get the underlying integer value for this set of flags */
	constexpr inline UnderlyingType operator+() noexcept { return flags; }

	/** Add a value to the set of flags */
	constexpr inline TFlags operator+(EnumType flag) const { return flags | (1 << (UnderlyingType)flag); }
	constexpr inline TFlags& operator+=(EnumType flag) { *this = *this + flag; return *this; }
	/** Remove a value from the set of flags */
	constexpr inline TFlags operator-(EnumType flag) const { return flags & ~(1 << (UnderlyingType)flag); }
	constexpr inline TFlags& operator-=(EnumType flag) { *this = *this - flag; return *this; }

	/** True if this set of flags contains no values */
	constexpr inline bool IsEmpty() const noexcept { return !!flags; }

	/** Returns the set of flags in this or the other */
	constexpr inline TFlags Union(TFlags other) const noexcept { return flags | other.flags; }
	/** Returns the set of flags in this but without the flags in other */
	constexpr inline TFlags Subtraction(TFlags other) const noexcept { return flags & ~other.flags; }
	/** Returns the set of common flags in both this and the other */
	constexpr inline TFlags Intersection(TFlags other) const noexcept { return flags & other.flags; }
	/** Returns the set of flags that are different in this and the other */
	constexpr inline TFlags Difference(TFlags other) const noexcept { return flags ^ other.flags; }

	/** True if this set of flags contains the value */
	constexpr inline bool Has(EnumType flag) const noexcept { return (flags & (1 << (UnderlyingType)flag)) != 0; }
	/** True if this set of flags contains all of the values in the other set */
	constexpr inline bool HasAll(TFlags other) const noexcept { return (flags & other.flags) == other.flags; }
	/** True if this set of flags contains any of the values in the other set */
	constexpr inline bool HasAny(TFlags other) const noexcept { return (flags & other.flags) != 0; }

protected:
	UnderlyingType flags;
};

template<typename EnumType>
const TFlags<EnumType> TFlags<EnumType>::None{0};

#define TFLAGS_METHODS(DerivedType)\
	using TFlags<DerivedType::EnumType>::TFlags;\
	DerivedType(TFlags<DerivedType::EnumType> const& other) : TFlags<DerivedType::EnumType>(other) {}
