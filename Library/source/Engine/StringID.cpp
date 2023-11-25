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
* 
*/

struct StringStorage {
	static constexpr size_t TableSize = 4096;
	static constexpr uint16_t PoolCapacity = std::numeric_limits<uint16_t>::max();

	struct Locator {
		uint16_t pool;
		uint16_t index;
	};

	struct View {
		View(uint16_t index, char const* string) : index(index), string(string) {}

		inline operator std::string_view() const { return std::string_view{ string, std::strlen(string) }; }

		inline bool operator==(std::string_view other) const {
			size_t const num = std::min<size_t>(PoolCapacity - index - 1, other.size());
			return std::strncmp(string, other.data(), num) == 0;
		}

		inline bool operator<(View other) const {
			return std::strcmp(string, other.string) < 0;
		}

	private:
		uint16_t index;
		char const* string;
	};

	std::vector<Locator>& GetLocators(uint16_t hash) {
		return table[hash % TableSize];
	}

	Locator GetLocator(StringID id) {
		return table[id.hash % TableSize][id.index];
	}

	View GetView(Locator locator) {
		return pools[locator.pool][locator.index];
	}

	Locator Store(std::string_view string) {
		//find an existing pool where we can store this string
		for (uint16_t poolIndex = 0; poolIndex < pools.size(); ++poolIndex) {
			if (auto const entryIndex = pools[poolIndex].Store(string)) {
				return Locator{ poolIndex, *entryIndex };
			}
		}

		if (pools.size() == std::numeric_limits<uint16_t>::max()) throw std::runtime_error{ "Cannot store new string, maximum number of string pools has been reached." };

		//Add a new pool where we can store this string
		auto const entryIndex = pools.emplace_back().Store(string);
		return Locator{ static_cast<uint16_t>(pools.size() - 1), *entryIndex };
	}

private:
	struct Pool {
	public:
		using CharacterBuffer = std::array<char, PoolCapacity>;

		Pool()
			: buffer(std::make_unique<CharacterBuffer>())
		{
			std::fill(buffer->begin(), buffer->end(), '\0');
		}
		Pool(Pool const&) = delete;
		Pool(Pool&&) = default;

		View operator[](uint16_t index) { return View{ index, &buffer->at(index) }; }

		inline size_t GetAvailable() const noexcept { return PoolCapacity - offset; }

		std::optional<uint16_t> Store(std::string_view string) {
			size_t const size = string.size() + static_cast<size_t>(!string.ends_with('\0'));
			if (size < GetAvailable()) {
				uint16_t const index = offset;
				offset += size;

				std::strncpy(&buffer->at(index), string.data(), size);
				return index;
			}
			return std::nullopt;
		}

	protected:
		std::unique_ptr<CharacterBuffer> buffer;
		uint16_t offset = 0;
	};

	std::array<std::vector<Locator>, TableSize> table;
	std::vector<Pool> pools;
};

StringStorage storage;

template<>
struct std::formatter<StringStorage::View> : std::formatter<std::string_view> {
	auto format(StringStorage::View p, format_context& ctx) const {
		return formatter<string_view>::format(p, ctx);
	}
};

const StringID StringID::None = "None"_sid;

StringID StringID::Find(std::string_view string) {
	uint16_t const hash = CreateHash(string);

	auto const& locators = storage.GetLocators(hash);

	for (uint16_t index = 0; index < locators.size(); ++index) {
		if (storage.GetView(locators[index]) == string) {
			return StringID{ hash, index, 0 };
		}
	}

	return StringID::None;
}

StringID::StringID(std::string_view string)
	: hash(CreateHash(string)), index(CreateIndex(hash, string))
{}

StringID::StringID(Initializer const& initializer) 
	: hash(initializer.hash), index(CreateIndex(initializer.hash, initializer.string))
{}

bool StringID::FastLess(StringID a, StringID b) {
	return std::make_tuple(a.hash, a.index, a.var) < std::make_tuple(b.hash, b.index, b.var);
}

bool StringID::LexicalLess(StringID a, StringID b) {
	return storage.GetView(storage.GetLocator(a)) < storage.GetView(storage.GetLocator(b));
}

std::string StringID::ToString() const {
	return std::format("{}{}", storage.GetView(storage.GetLocator(*this)), var);
}

std::string_view StringID::ToStringView() const {
	return t_printf("%s%i", storage.GetView(storage.GetLocator(*this)), var);
}

uint16_t StringID::CreateIndex(uint16_t hash, std::string_view string) {
	auto& locators = storage.GetLocators(hash);

	//Find an existing locator that matches the input string
	for (size_t index = 0; index < locators.size(); ++index) {
		if (storage.GetView(locators[index]) == string) {
			return index;
		}
	}

	//Create a new locator for the input string by storing it in the pools
	if (locators.size() == std::numeric_limits<uint16_t>::max()) throw std::runtime_error{ "Too many collisions for hash" };
	locators.emplace_back(storage.Store(string));
	return locators.size() - 1;
}
