#pragma once
#include "Engine/Hash.h"
#include "Engine/StandardTypes.h"

/** Represents a stable and cheap identifier based on a string, but which internally caches strings to avoid keeping many duplicates in memory. This also means comparisons are very fast. */
struct StringID {
	/** Initializer created for string IDs known at compile time, which moves some of the initialization cost to compile time */
	struct Initializer {
	private:
		friend struct StringID;
		friend consteval Initializer operator ""_sid(char const* p, size_t s);

		constexpr Initializer(std::string_view string) : hash(CreateHash(string)), string(string) {}

		uint16_t hash;
		std::string_view string;
	};

	static const StringID None;

	/** Find the StringID for the string if it already exists, or StringID::None if it doesn't already exist. */
	static StringID Find(std::string_view string);

	StringID(std::string_view string);
	StringID(Initializer const& initializer);
	StringID(StringID const&) = default;

	inline StringID& operator=(std::string_view string) { *this = StringID{ string }; return *this; }

	inline bool operator==(StringID const& other) const { return hash == other.hash && index == other.index && var == other.var; }
	inline bool operator!=(StringID const& other) const { return !this->operator==(other); }

	inline StringID& operator+=(uint32_t offset) { var += offset; return *this; }
	inline StringID& operator-=(uint32_t offset) { var -= offset; return *this; }
	inline StringID operator+(uint32_t offset) const { return StringID{ hash, index, var + offset }; }
	inline StringID operator-(uint32_t offset) const { return StringID{ hash, index, var - offset }; }
	inline StringID operator++(int) const { return *this + 1; }
	inline StringID operator--(int) const { return *this - 1; }

	inline StringID& operator++() { ++var; return *this; }
	inline StringID& operator--() { --var; return *this; }

	/** Implements operator< by performing a very fast comparison of internal values. Fast sorting, but string order may seem arbitrary to a user. */
	static bool FastLess(StringID a, StringID b);
	/** Implements operator< by performing a slower comparison on the actual string contents. Slow sorting, but strings appear in ascending order. */
	static bool LexicalLess(StringID a, StringID b);

	/** Returns the string that corresponds to this StringID */
	std::string ToString() const;
	/** Returns the string that corresponds to this StringID as a view to a temporary string */
	std::string_view ToStringView() const;

private:
	friend struct StringStorage;

	static constexpr uint16_t CreateHash(std::string_view string) {
		uint32_t const full = Hash32{ string }.ToValue();
		return static_cast<uint16_t>(full) ^ static_cast<uint16_t>(full >> 16);
	}
	static uint16_t CreateIndex(uint16_t hash, std::string_view string);

	StringID(uint16_t hash, uint16_t index, uint32_t var) : hash(hash), index(index), var(var) {}

	uint16_t hash;
	uint16_t index;
	uint32_t var;
};

consteval StringID::Initializer operator ""_sid(char const* p, size_t s) { return StringID::Initializer{ std::string_view{ p, s } }; };
