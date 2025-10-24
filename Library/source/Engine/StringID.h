#pragma once
#include "Engine/Core.h"
#include "Engine/Hash.h"
#include "Engine/Reflection.h"
#include "Engine/StringUtils.h"
#include "Engine/StringView.h"

/** A stable and cheap identifier based on a string, but which internally caches strings to avoid keeping many duplicates in memory. This also means comparisons are very fast. */
struct StringID {
	/** Initializer created for string IDs known at compile time, which moves some of the initialization cost to compile time. Can only be created with the "_sid" literal suffix. */
	struct Initializer {
	private:
		friend struct StringID;
		friend consteval Initializer operator ""_sid(char const* p, size_t s);

		constexpr Initializer(StringUtils::DecomposedString const& source) : source(source), hash(CreateHash(source.body)) {
			if (ContainsInvalidCharacters(source.body)) throw std::runtime_error{ "Source string contains invalid characters" };
		}
		constexpr Initializer(std::string_view string) : Initializer(StringUtils::DecomposedString{ string }) {}

		StringUtils::DecomposedString source;
		uint16_t hash = 0;
	};

	/** Reflected type information */
	static const Reflection::TStructTypeInfo<StringID> info_StringID;

	/** Constant for a fully numeric StringID representing zero */
	static const StringID Zero;
	/** Constant for a StringID representing "None" */
	static const StringID None;
	/** Constant for a StringID representing "Temporary" */
	static const StringID Temporary;

	/** Find the StringID for the string if it already exists */
	static std::optional<StringID> Find(std::string_view string);

	StringID(std::string_view string) : StringID(StringUtils::DecomposedString{ string }) {}
	StringID(struct Initializer const& initializer);
	StringID(StringID const&) = default;

	inline StringID& operator=(std::string_view string) { *this = StringID{ string }; return *this; }

	inline bool operator==(StringID const& other) const { return hash == other.hash && collision == other.collision && var == other.var; }
	inline bool operator!=(StringID const& other) const { return !this->operator==(other); }

	inline StringID& operator+=(uint32_t offset) { var += offset; return *this; }
	inline StringID& operator-=(uint32_t offset) { var -= offset; return *this; }
	inline StringID operator+(uint32_t offset) const { return StringID{ hash, collision, var + offset }; }
	inline StringID operator-(uint32_t offset) const { return StringID{ hash, collision, var - offset }; }
	inline StringID operator++(int) const { return *this + 1; }
	inline StringID operator--(int) const { return *this - 1; }

	inline StringID& operator++() { ++var; return *this; }
	inline StringID& operator--() { --var; return *this; }

	/** Returns true if the string contains invalid characters that cannot be part of a source string for a StringID. */
	static constexpr bool ContainsInvalidCharacters(std::string_view string) {
		return string.find_first_of("\0\'\"\?\\\a\b\f\n\r\t\v:&|/"sv) != std::string_view::npos;
	}

	/** Implements operator< by performing a very fast comparison of internal values. Fast sorting, but string order may seem arbitrary to a user. */
	static bool FastLess(StringID a, StringID b);
	/** Implements operator< by performing a slower comparison on the actual string contents. Slow sorting, but strings appear in ascending order. */
	static bool LexicalLess(StringID a, StringID b);

	/** Returns the string that corresponds to this StringID */
	std::string ToString() const;
	/** Returns the string that corresponds to this StringID as a view to a temporary string */
	std::string_view ToStringView() const;

private:
	friend struct std::hash<StringID>;
	friend struct StringStorage;
	friend struct YAML::as_if<StringID, void>;
	
	static constexpr uint16_t CreateHash(std::string_view string) {
		uint32_t const full = Hash32{ string }.ToValue();
		return static_cast<uint16_t>(full) ^ static_cast<uint16_t>(full >> 16);
	}
	static uint16_t EmplaceString(uint16_t hash, std::string_view string);

	//Default construction is only available in specific contexts where a library requires a default constructor. In those cases, it's equivalent to constructing from StringID::None.
	StringID() : StringID(StringID::None) {}

	StringID(const StringUtils::DecomposedString& source);
	StringID(uint16_t hash, uint16_t collision, uint32_t var) : hash(hash), collision(collision), var(var) {}

	uint16_t hash;
	uint16_t collision;
	uint32_t var;
};

static_assert(sizeof(StringID) == sizeof(uint64_t), "StringID should be exactly 8 bytes in size, directly convertible to a uint64_t");

consteval StringID::Initializer operator ""_sid(char const* p, size_t s) { return StringID::Initializer{ std::string_view{ p, s } }; };

/** A default less-than operator that uses a fast comparison. See StringID::LexicalLess for another version. */
inline bool operator<(StringID const& a, StringID const& b) { return StringID::FastLess(a, b); }

template<>
struct std::hash<StringID> {
	size_t operator()(StringID id) const {
		return static_cast<size_t>(id.hash) | (static_cast<size_t>(id.collision) << 16) | (static_cast<size_t>(id.var) << 32);
	}
};

template<>
struct std::formatter<StringID> : std::formatter<std::string_view> {
	auto format(const StringID& sid, format_context& ctx) const {
		return formatter<string_view>::format(sid.ToStringView(), ctx);
	}
};

template<>
struct Reflect<StringID> {
	static Reflection::StructTypeInfo const& Get() { return StringID::info_StringID; }
	static constexpr Hash128 ID = Hash128{ std::string_view{ STRINGIFY(StringID) } };
};

namespace Archive {
	template<>
	struct Serializer<StringID> {
		static void Write(Output& archive, StringID const sid);
		static void Read(Input& archive, StringID& sid);
	};

	template<>
	inline StringID DefaultReadValue<StringID>() { return StringID::None; }
}

namespace YAML {
	template<>
	struct convert<StringID> {
		static Node encode(StringID id);
		static bool decode(Node const& node, StringID& id);
	};
}
