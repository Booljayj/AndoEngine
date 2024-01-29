#pragma once
#include <type_traits>

/** Test if the bitflag value is set to true in the mask */
#define TEST_BIT(Mask, Flag) ((size_t)(Mask) & (size_t)(Flag))
/** Set the bitflag to true in the mask */
#define SET_BIT(Mask, Flag) ((size_t)(Mask) |= (size_t)(Flag))
/** Set the bitflag to false in the mask */
#define CLEAR_BIT(Mask, Flag) ((size_t)(Mask) &= (size_t)(~(Flag)))

template<stdext::enumeration InEnumType>
struct TFlags {
public:
	using EnumType = InEnumType;
	using UnderlyingType = typename std::underlying_type_t<EnumType>;

	constexpr inline TFlags() noexcept = default;
	constexpr inline TFlags(TFlags const& other) noexcept = default;
	constexpr inline TFlags(TFlags&& other) noexcept = default;

	constexpr inline TFlags(UnderlyingType flags) : flags(flags) {}
	constexpr inline TFlags(EnumType flag) : flags(1 << static_cast<UnderlyingType>(flag)) {}
	constexpr inline TFlags(std::initializer_list<EnumType> inFlags) {
		for (EnumType flag : inFlags) flags |= (1 << (UnderlyingType)flag);
	}

	template<typename... FlagTypes>
		requires std::conjunction_v<std::is_same<FlagTypes, EnumType>...>
	static constexpr inline TFlags Make(FlagTypes... flags) {
		return TFlags{ static_cast<UnderlyingType>((... | (1 << static_cast<UnderlyingType>(flags)))) };
	}

	static constexpr inline TFlags None() { return TFlags{}; }
	
	constexpr inline TFlags& operator=(TFlags const& other) noexcept = default;
	constexpr inline TFlags& operator=(TFlags&& other) noexcept = default;

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
	/** Returns the set of flags in both this and other */
	constexpr inline TFlags Intersection(TFlags other) const noexcept { return flags & other.flags; }
	/** Returns the set of flags that are different in this and other */
	constexpr inline TFlags Difference(TFlags other) const noexcept { return flags ^ other.flags; }

	/** True if this set of flags contains the value */
	constexpr inline bool Has(EnumType flag) const noexcept { return (flags & (1 << (UnderlyingType)flag)) != 0; }
	/** True if this set of flags contains all of the values in the other set */
	constexpr inline bool HasAll(TFlags other) const noexcept { return (flags & other.flags) == other.flags; }
	/** True if this set of flags contains any of the values in the other set */
	constexpr inline bool HasAny(TFlags other) const noexcept { return (flags & other.flags) != 0; }

	/** True if this set of flags contains all of the values */
	template<typename... FlagTypes>
	constexpr inline bool HasAll(EnumType flag, FlagTypes... other) { return HasAll(Make(flag, other...)); }
	/** True if this set of flags contains any of the values */
	template<typename... FlagTypes>
	constexpr inline bool HasAny(EnumType flag, FlagTypes... other) { return HasAny(Make(flag, other...)); }

protected:
	UnderlyingType flags = 0;
};

#define TFLAGS_METHODS(DerivedType)\
	using TFlags<EnumType>::TFlags;\
	DerivedType(TFlags<EnumType> const& other) : TFlags<EnumType>(other) {}\
	operator TFlags<EnumType>() const { return TFlags<EnumType>{ flags }; }
