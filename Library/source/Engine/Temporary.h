#pragma once
#include "Engine/Allocators.h"
#include "Engine/Buffers.h"
#include "Engine/StandardTypes.h"

/**
 * Temporaries are containers allocated from a per-thread linear buffer. They are fast, cheap, and flexible.
 * All temporaries created within a scope marked with SCOPED_TEMPORARIES() are stable and valid within that scope.
 * When exiting the scope, the memory used by temporaries will be repurposed for new temporaries.
 */

//============================================================
// Buffer types

/** A buffer used for temporary allocations within a thread. Assigned to the thread in which it is created. */
struct ThreadBuffer : public HeapBuffer {
public:
	ThreadBuffer(size_t capacity);
	~ThreadBuffer();

	static inline ThreadBuffer& Get() { return *current; }
	static void LogDebugStats();

private:
	static thread_local ThreadBuffer* current;
};

/** A mark which will save the temporary buffer's current cursor position when created, and set the cursor to that position when destroyed. */
struct ScopedThreadBufferMark : public HeapBuffer::ScopedMark {
	ScopedThreadBufferMark();
};

//============================================================
// Temporary allocator types

/** Allocator used for temporary allocations */
template<typename T>
struct TTemporaryAllocator : public TLinearBufferAllocator<T, std::allocator<T>> {
public:
	TTemporaryAllocator()
		: TLinearBufferAllocator<T, std::allocator<T>>(ThreadBuffer::Get())
	{}

	TTemporaryAllocator(TTemporaryAllocator const&) = default;
	template<typename U> TTemporaryAllocator(TTemporaryAllocator<U> const& other) : TLinearBufferAllocator<T>(other) {}

	template<typename U>
	struct rebind {
		using other = TTemporaryAllocator<U>;
	};
};

//============================================================
// Temporary string types

template<typename CharType>
using t_basic_string = std::basic_string<CharType, std::char_traits<CharType>, TTemporaryAllocator<CharType>>;

using t_string = t_basic_string<char>;
using t_wstring = t_basic_string<wchar_t>;
using t_u16string = t_basic_string<char16_t>;
using t_u32string = t_basic_string<char32_t>;

template<typename... ArgTypes>
std::string_view Format(std::format_string<ArgTypes...> format, ArgTypes&& ... args) {
	using namespace std;

	Buffer& buffer = ThreadBuffer::Get();
	size_t const available = buffer.GetAvailable();

	char* const begin = buffer.GetCursor();
	auto const result = format_to_n(back_inserter(buffer), available, format, forward<ArgTypes>(args)...);
	
	return string_view{ begin, static_cast<uint32_t>(result.size) };
}

/** A type that can be constructed with only a string parameter */
template<typename T>
concept ConstructibleFromString =
	std::constructible_from<std::string_view> or
	std::constructible_from<std::string> or
	std::constructible_from<char const*>;

/** Given a type that can be constructed with a string, construct it with a formatted string created with the temporary allocator */
template<ConstructibleFromString ReturnType, typename... ArgTypes>
constexpr ReturnType FormatType(std::format_string<ArgTypes...> format, ArgTypes&&... arguments) {
	using namespace std;

	if constexpr (sizeof...(ArgTypes) > 0) {
		//One or more formatting parameters are present, so we need to perform formatting
		const auto view = Format(format, forward<ArgTypes>(arguments)...);

		if constexpr (constructible_from<ReturnType, string_view>) return ReturnType{ view };
		else if constexpr (constructible_from<ReturnType, string>) return ReturnType{ string{ view } };
		else return ReturnType{ view.data() };
	
	} else {
		//No formatting parameters are present, so we can skip formatting and just use the provided format string
		if constexpr (constructible_from<ReturnType, string_view>) return ReturnType{ format.get() };
		else if constexpr (constructible_from<ReturnType, string>) return ReturnType{ string{ format.get() } };
		else return ReturnType{ format.get().data() };
	}
}

//============================================================
// Temporary container types

// Sequence Containers
template<class T>
using t_vector = std::vector<T, TTemporaryAllocator<T>>;
template<class T>
using t_deque = std::deque<T, TTemporaryAllocator<T>>;
template<class T>
using t_forward_list = std::forward_list<T, TTemporaryAllocator<T>>;
template<class T>
using t_list = std::list<T, TTemporaryAllocator<T>>;

// Associative Containers
template<class T>
using t_set = std::set<T, std::less<T>, TTemporaryAllocator<T>>;
template<class T>
using t_multiset = std::multiset<T, std::less<T>, TTemporaryAllocator<T>>;
template<class KeyType, class ValueType>
using t_map = std::map<KeyType, ValueType, std::less<KeyType>, TTemporaryAllocator<std::pair<const KeyType, ValueType>>>;
template<class KeyType, class ValueType>
using t_multimap = std::multimap<KeyType, ValueType, std::less<KeyType>, TTemporaryAllocator<std::pair<const KeyType, ValueType>>>;

// Unordered Associative Containers
template<class T>
using t_unordered_set = std::unordered_set<T, std::hash<T>, std::equal_to<T>, TTemporaryAllocator<T>>;
template<class T>
using t_unordered_multiset = std::unordered_multiset<T, std::hash<T>, std::equal_to<T>, TTemporaryAllocator<T>>;
template<class KeyType, class ValueType>
using t_unordered_map = std::unordered_map<KeyType, ValueType, std::hash<KeyType>, std::equal_to<KeyType>, TTemporaryAllocator<std::pair<const KeyType, ValueType>>>;
template<class KeyType, class ValueType>
using t_unordered_multimap = std::unordered_multimap<KeyType, ValueType, std::hash<KeyType>, std::equal_to<KeyType>, TTemporaryAllocator<std::pair<const KeyType, ValueType>>>;

// Container Adapters
template<class T>
using t_stack = std::stack<T, t_deque<T>>;
template<class T>
using t_queue = std::queue<T, t_deque<T>>;
template<class T>
using t_priority_queue = std::priority_queue<T, t_deque<T>>;
