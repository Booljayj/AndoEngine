#pragma once
#include "Engine/Concepts.h"
#include "Engine/Threads.h"

/** Test if the bitflag value is set to true in the mask */
#define TEST_BIT(Mask, Flag) ((size_t)(Mask) & (size_t)(Flag))
/** Set the bitflag to true in the mask */
#define SET_BIT(Mask, Flag) ((size_t)(Mask) |= (size_t)(Flag))
/** Set the bitflag to false in the mask */
#define CLEAR_BIT(Mask, Flag) ((size_t)(Mask) &= (size_t)(~(Flag)))

struct NoFlagsInitializer {};
/**
 * Constant that can be used to initialize a set of flags that contains no flags.
 * This is the same as default-initializing the flags, but can be shorter in some cases. Similar to std::nullopt.
 */
constexpr NoFlagsInitializer NoFlags;

/** CRTP class that injects methods to make DerivedType behave as a set of flags based on the enum type */
template<Concepts::Enumeration InEnumType>
struct TFlags {
public:
	using EnumType = InEnumType;
	using UnderlyingType = typename std::underlying_type_t<EnumType>;

	constexpr inline TFlags() noexcept = default;
	constexpr inline TFlags(TFlags const& other) noexcept = default;
	constexpr inline TFlags(TFlags&& other) noexcept = default;

	constexpr inline TFlags(NoFlagsInitializer) : TFlags() {}
	constexpr inline TFlags(UnderlyingType flags) : flags(flags) {}
	constexpr inline TFlags(EnumType flag) : flags(1 << static_cast<UnderlyingType>(flag)) {}
	constexpr inline TFlags(std::initializer_list<EnumType> flags_list) {
		for (EnumType flag : flags_list) flags |= (1 << static_cast<UnderlyingType>(flag));
	}

	constexpr inline TFlags& operator=(TFlags const& other) noexcept = default;

	constexpr inline bool operator==(TFlags other) const noexcept { return flags == other.flags; }
	constexpr inline bool operator!=(TFlags other) const noexcept { return flags != other.flags; }
	 
	/** Get the underlying integer value for this set of flags */
	constexpr inline UnderlyingType operator+() const noexcept { return flags; }

	/** Add a value to the set of flags */
	template<typename Derived>
	constexpr inline Derived operator+(this Derived const& self, EnumType flag) { return self.flags | (1 << (UnderlyingType)flag); }
	template<typename Derived>
	constexpr inline Derived& operator+=(this Derived& self, EnumType flag) { self = self + flag; return self; }
	/** Remove a value from the set of flags */
	template<typename Derived>
	constexpr inline Derived operator-(this Derived const& self, EnumType flag) { return self.flags & ~(1 << (UnderlyingType)flag); }
	template<typename Derived>
	constexpr inline Derived& operator-=(this Derived& self, EnumType flag) { self = self - flag; return self; }

	/** True if this set of flags contains no values */
	constexpr inline bool IsEmpty() const noexcept { return !!flags; }

	/** Returns the set of flags in this or the other */
	template<typename Derived>
	constexpr inline Derived Union(this Derived const& self, TFlags other) noexcept { return self.flags | other.flags; }
	/** Returns the set of flags in this but without the flags in other */
	template<typename Derived>
	constexpr inline Derived Subtraction(this Derived const& self, TFlags other) noexcept { return self.flags & ~other.flags; }
	/** Returns the set of flags in both this and other */
	template<typename Derived>
	constexpr inline Derived Intersection(this Derived const& self, TFlags other) noexcept { return self.flags & other.flags; }
	/** Returns the set of flags that are different in this and other */
	template<typename Derived>
	constexpr inline Derived Difference(this Derived const& self, TFlags other) noexcept { return self.flags ^ other.flags; }

	/** True if this set of flags contains the value */
	constexpr inline bool Has(EnumType flag) const noexcept { return (flags & (1 << (UnderlyingType)flag)) != 0; }
	/** True if this set of flags contains all of the values in the other set */
	constexpr inline bool HasAll(TFlags other) const noexcept { return (flags & other.flags) == other.flags; }
	/** True if this set of flags contains any of the values in the other set */
	constexpr inline bool HasAny(TFlags other) const noexcept { return (flags & other.flags) != 0; }

	/** True if this set of flags contains all of the values */
	template<typename... FlagTypes>
	constexpr inline bool HasAll(EnumType flag, FlagTypes... other) const { return HasAll(TFlags{ flag, other... }); }
	/** True if this set of flags contains any of the values */
	template<typename... FlagTypes>
	constexpr inline bool HasAny(EnumType flag, FlagTypes... other) const { return HasAny(TFlags{ flag, other... }); }

protected:
	UnderlyingType flags = 0;
};

#define DEFINE_FLAGS_STRUCT(TypeName) struct F ## TypeName : TFlags<E ## TypeName>

template<Concepts::Enumeration InEnumType>
struct TAtomicFlags {
public:
	using EnumType = InEnumType;
	using UnderlyingType = typename std::underlying_type_t<EnumType>;

	constexpr inline TAtomicFlags() noexcept = default;
	constexpr inline TAtomicFlags(TAtomicFlags const& other) noexcept = default;
	constexpr inline TAtomicFlags(TAtomicFlags&& other) noexcept = default;

	constexpr inline TAtomicFlags(UnderlyingType flags) : flags(flags) {}
	constexpr inline TAtomicFlags(EnumType flag) : flags(1 << static_cast<UnderlyingType>(flag)) {}
	constexpr inline TAtomicFlags(std::initializer_list<EnumType> inFlags) {
		UnderlyingType combined = 0;
		for (EnumType flag : inFlags) combined |= (1 << (UnderlyingType)flag);
		flags.store(combined);
	}

	template<typename... FlagTypes>
		requires std::conjunction_v<std::is_same<FlagTypes, EnumType>...>
	static constexpr inline TAtomicFlags Make(FlagTypes... flags) {
		return TAtomicFlags{ static_cast<UnderlyingType>((... | (1 << static_cast<UnderlyingType>(flags)))) };
	}

	static constexpr inline TAtomicFlags None() { return TAtomicFlags{}; }

	constexpr inline TAtomicFlags& operator=(TAtomicFlags const& other) noexcept = default;
	constexpr inline TAtomicFlags& operator=(TAtomicFlags&& other) noexcept = default;

	constexpr inline bool operator==(TAtomicFlags other) const noexcept { return flags == other.flags; }
	constexpr inline bool operator!=(TAtomicFlags other) const noexcept { return flags != other.flags; }

	/** Get the underlying integer value for this set of flags */
	constexpr inline UnderlyingType operator+() noexcept { return flags; }

	/** Add a value to the set of flags */
	constexpr inline TAtomicFlags operator+(EnumType flag) const { return flags | (1 << (UnderlyingType)flag); }
	constexpr inline TAtomicFlags& operator+=(EnumType flag) {
		UnderlyingType expected = flags.load();
		while (!flags.compare_exchange_weak(expected, expected | (1 << (UnderlyingType)flag))) {}
		return *this;
	}
	/** Remove a value from the set of flags */
	constexpr inline TAtomicFlags operator-(EnumType flag) const { return flags & ~(1 << (UnderlyingType)flag); }
	constexpr inline TAtomicFlags& operator-=(EnumType flag) {
		UnderlyingType expected = flags.load();
		while (!flags.compare_exchange_weak(expected, expected & ~(1 << (UnderlyingType)flag))) {}
		return *this;
	}

	/** True if this set of flags contains no values */
	constexpr inline bool IsEmpty() const noexcept { return !!flags; }

	/** Returns the set of flags in this or the other */
	constexpr inline TAtomicFlags Union(TAtomicFlags other) const noexcept { return flags | other.flags; }
	/** Returns the set of flags in this but without the flags in other */
	constexpr inline TAtomicFlags Subtraction(TAtomicFlags other) const noexcept { return flags & ~other.flags; }
	/** Returns the set of flags in both this and other */
	constexpr inline TAtomicFlags Intersection(TAtomicFlags other) const noexcept { return flags & other.flags; }
	/** Returns the set of flags that are different in this and other */
	constexpr inline TAtomicFlags Difference(TAtomicFlags other) const noexcept { return flags ^ other.flags; }

	/** True if this set of flags contains the value */
	constexpr inline bool Has(EnumType flag) const noexcept { return (flags & (1 << (UnderlyingType)flag)) != 0; }
	/** True if this set of flags contains all of the values in the other set */
	constexpr inline bool HasAll(TAtomicFlags other) const noexcept { return (flags & other.flags) == other.flags; }
	/** True if this set of flags contains any of the values in the other set */
	constexpr inline bool HasAny(TAtomicFlags other) const noexcept { return (flags & other.flags) != 0; }

	/** True if this set of flags contains all of the values */
	template<typename... FlagTypes>
	constexpr inline bool HasAll(EnumType flag, FlagTypes... other) { return HasAll(Make(flag, other...)); }
	/** True if this set of flags contains any of the values */
	template<typename... FlagTypes>
	constexpr inline bool HasAny(EnumType flag, FlagTypes... other) { return HasAny(Make(flag, other...)); }

protected:
	std::atomic<UnderlyingType> flags = 0;
};

#define DEFINE_ATOMIC_FLAGS_STRUCT(TypeName) struct F ## TypeName : TAtomicFlags<E ## TypeName, F ## TypeName>

#define TFLAGS_METHODS(DerivedType)\
	using TFlags<EnumType, DerivedType>::TFlags;\

