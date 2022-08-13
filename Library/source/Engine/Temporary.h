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
// Basic temporary support

/** The pointer to the buffer for temporary allocations in each thread. */
extern thread_local Buffer* threadTemporaryBuffer;

/** Assign the temporary buffer that should be used by the current thread. This should be called when creating the thread, before using any temporaries */
inline void AssignThreadTemporaryBuffer(Buffer& buffer) {
	assert(!threadTemporaryBuffer);
	threadTemporaryBuffer = &buffer;
}

/** When exiting this scope, all memory used by temporaries inside the scope will be repurposed. */
#define SCOPED_TEMPORARIES() HeapBuffer::Mark tempMark_ ## __COUNTER__{ *threadTemporaryBuffer }

/** Allocator used for temporary allocations */
template<typename T>
struct TTemporaryAllocator : public TLinearBufferAllocator<T> {
public:
	TTemporaryAllocator()
	: TLinearBufferAllocator<T>(*threadTemporaryBuffer)
	{}
};

//============================================================
// Temporary string types

template<typename CharType>
using t_basic_string = std::basic_string<CharType, std::char_traits<CharType>, TTemporaryAllocator<CharType>>;

using t_string = t_basic_string<char>;
using t_wstring = t_basic_string<wchar_t>;
using t_u16string = t_basic_string<char16_t>;
using t_u32string = t_basic_string<char32_t>;

std::string_view t_printf(char const* format, ...);

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
