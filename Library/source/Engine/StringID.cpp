#include "Engine/StringID.h"
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"

/*
Given a string input:
1. create a hash from the string
2. modulo the hash to get the index for the string (define max possible number of strings)
3. index is used to find a std::vector of locators. Locators point to where the string is stored in an pooled allocator.
4. iterate through locations and check them to see if they match the string
5. If so, we have the string stored already. If not and we need to store it, use the pooled allocator to store the string and put the location in the vector
6. Return the hash and the index in the vector, which together can be used to quickly find the locator again

Pooled allocator has the following requirements:
- Allocations are forward-only, deallocation will never happen
- Allocation should be for blocks that hold a certain number of maximumal-length strings
- Blocks hold as many strings as possible given their size
- Allocations are first-come, first-serve, and do not depend on the content of the strings
- Allocations should be indentifiable using a locator, which identifies the block and the offset within the block where a string is located.
*/

struct StringStorage {
	static constexpr size_t TableSize = 4096;
	static constexpr uint16_t PoolCapacity = std::numeric_limits<uint16_t>::max();

	/** Locator which points to a specific string stored in the string pools */
	struct Locator {
		uint16_t pool;
		uint16_t offset;
	};

	/** A contiguous collection of locators */
	struct LocatorCollection {
		static constexpr size_t NumReservedLocators = 4;

		LocatorCollection() {
			container.reserve(NumReservedLocators);
		}

		inline size_t size() const { return container.size(); }
		inline Locator const& operator[](size_t index) const { return container[index]; }
		inline void emplace_back(Locator locator) { container.emplace_back(locator); }
		
	private:
		std::vector<Locator> container;
	};

	/** View type that represents a string in the the string pools. Implements operators for efficient comparisons and conversions. */
	struct View {
		View(uint16_t offset, char const* string) : offset(offset), string(string) {}

		inline operator std::string_view() const { return std::string_view{ string, std::strlen(string) }; }

		inline bool operator==(std::string_view other) const {
			//We'll use the available capacity as the size of the view here, which saves an extra strlen calculation.
			//Since we're using the min of the two sizes, this always results in the correct comparison anyways.
			size_t const num = std::min<size_t>(PoolCapacity - offset - 1, other.size());
			return std::strncmp(string, other.data(), num) == 0;
		}

		inline bool operator<(View other) const { return std::strcmp(string, other.string) < 0; }

	private:
		uint16_t offset;
		char const* string;
	};

	/** A pool of strings, which are stored in increasing positions until the pool has no more available space for more strings. */
	struct Pool {
		using CharacterBuffer = std::array<char, PoolCapacity>;

		Pool() : buffer(std::make_unique<CharacterBuffer>()) {
			std::fill(buffer->begin(), buffer->end(), '\0');
		}
		Pool(Pool const&) = delete;
		Pool(Pool&&) = default;

		inline View operator[](uint16_t offset) const { return View{ offset, &buffer->at(offset) }; }

		inline size_t GetAvailable() const noexcept { return PoolCapacity - currentOffset; }

		std::optional<uint16_t> Store(std::string_view string) {
			size_t const size = string.size();
			if (size < GetAvailable()) {
				uint16_t const offset = currentOffset;
				currentOffset += size;

				std::strncpy(&buffer->at(offset), string.data(), size);
				return offset;
			}
			return std::nullopt;
		}

	protected:
		std::unique_ptr<CharacterBuffer> buffer;
		uint16_t currentOffset = 0;
	};

	std::array<LocatorCollection, TableSize> table;
	std::vector<Pool> pools;

	LocatorCollection& GetLocators(uint16_t hash) { return table[hash % TableSize]; }
	Locator GetLocator(StringID id) const { return table[id.hash % TableSize][id.collision]; }
	View GetView(Locator locator) const { return pools[locator.pool][locator.offset]; }

	Locator Store(std::string_view string) {
		//find an existing pool where we can store this string
		for (uint16_t poolIndex = 0; poolIndex < pools.size(); ++poolIndex) {
			if (auto const offset = pools[poolIndex].Store(string)) return Locator{ poolIndex, offset.value() };
		}

		if (pools.size() == std::numeric_limits<uint16_t>::max()) throw MakeException<std::runtime_error>("Cannot store new string, maximum number of string pools has been reached.");

		//Add a new pool where we can store this string
		auto const entryIndex = pools.emplace_back().Store(string);
		return Locator{ static_cast<uint16_t>(pools.size() - 1), *entryIndex };
	}

private:
	//Diagnostic size values
	static constexpr size_t TotalTableBytesDirect = sizeof(table) + (TableSize * sizeof(Locator) * LocatorCollection::NumReservedLocators);
	static constexpr size_t TotalPoolBytes = sizeof(Pool) + sizeof(Pool::CharacterBuffer);
};

StringStorage storage;

template<>
struct std::formatter<StringStorage::View> : std::formatter<std::string_view> {
	auto format(StringStorage::View const& p, format_context& ctx) const {
		return formatter<string_view>::format(p, ctx);
	}
};

const Reflection::TPrimitiveTypeInfo<StringID> StringID::info_StringID = Reflection::TPrimitiveTypeInfo<StringID>{ "StringID"sv }
	.Description("Stable and fast identifier created from a string, which can be converted back into the original string"sv);

const StringID StringID::Zero = "0"_sid;
const StringID StringID::None = "None"_sid;
const StringID StringID::Temporary = "Temporary"_sid;

std::optional<StringID> StringID::Find(std::string_view string) {
	uint16_t const hash = CreateHash(string);

	auto const& locators = storage.GetLocators(hash);

	for (uint16_t index = 0; index < locators.size(); ++index) {
		if (storage.GetView(locators[index]) == string) return StringID{ hash, index, 0 };
	}

	return std::nullopt;
}

StringID::StringID(Initializer const& initializer)
	: hash(initializer.hash), collision(EmplaceString(initializer.hash, initializer.source.body)), var(initializer.source.suffix)
{}

StringID::StringID(const StringUtils::DecomposedString& source)
	: hash(CreateHash(source.body)), collision(EmplaceString(hash, source.body)), var(source.suffix)
{}

bool StringID::FastLess(StringID a, StringID b) {
	return std::make_tuple(a.hash, a.collision, a.var) < std::make_tuple(b.hash, b.collision, b.var);
}

bool StringID::LexicalLess(StringID a, StringID b) {
	return storage.GetView(storage.GetLocator(a)) < storage.GetView(storage.GetLocator(b));
}

std::string StringID::ToString() const {
	return std::format("{}{}", storage.GetView(storage.GetLocator(*this)), var);
}

std::string_view StringID::ToStringView() const {
	return format_temp("{}{}", storage.GetView(storage.GetLocator(*this)), var);
}

uint16_t StringID::EmplaceString(uint16_t hash, std::string_view string) {
	auto& locators = storage.GetLocators(hash);

	//Find an existing locator that matches the input string
	for (size_t index = 0; index < locators.size(); ++index) {
		if (storage.GetView(locators[index]) == string) return index;
	}

	//Create a new locator for the input string by storing it in the pools
	if (locators.size() == std::numeric_limits<uint16_t>::max()) throw MakeException<std::runtime_error>("Too many collisions for hash");
	locators.emplace_back(storage.Store(string));
	return locators.size() - 1;
}
