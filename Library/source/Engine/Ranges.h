#pragma once
#include <ranges>
#include <span>
#include <stdexcept>
#include "Engine/Core.h"

namespace ranges = std::ranges;

namespace stdext {
	/** Convert a byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T>
	constexpr std::span<T const> from_bytes(std::span<std::byte const> bytes) {
		if (bytes.size() % sizeof(T) != 0) throw std::runtime_error{ "invalid conversion, source byte span cannot be converted into a whole number of objects of the target type" };
		return std::span<T const>{ reinterpret_cast<T const*>(bytes.data()), bytes.size() / sizeof(T) };
	}
	/** Convert a byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T, size_t NumBytes>
		requires (NumBytes % sizeof(T) == 0)
	constexpr std::span<T const, NumBytes / sizeof(T)> from_bytes(std::span<std::byte const, NumBytes> bytes) {
		return std::span<T const, NumBytes / sizeof(T)>{ reinterpret_cast<T const*>(bytes.data()), bytes.size() / sizeof(T) };
	}
	/** Convert a writable byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T>
	constexpr std::span<T> from_writable_bytes(std::span<std::byte> bytes) {
		if (bytes.size() % sizeof(T) != 0) throw std::runtime_error{ "invalid conversion, source byte span cannot be converted into a whole number of objects of the target type" };
		return std::span<T>{ reinterpret_cast<T*>(bytes.data()), bytes.size() / sizeof(T) };
	}
	/** Convert a writable byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T, size_t NumBytes>
		requires (NumBytes % sizeof(T) == 0)
	constexpr std::span<T, NumBytes / sizeof(T)> from_writable_bytes(std::span<std::byte, NumBytes> bytes) {
		return std::span<T, NumBytes / sizeof(T)>{ reinterpret_cast<T*>(bytes.data()), bytes.size() / sizeof(T) };
	}
}

namespace Algo {
	constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();

	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, size_t index) {
		const size_t size = ranges::size(range);
		if (index < size) {
			if (index < (size - 1)) std::swap(range[index], range.back());
			range.pop_back();
			return true;
		}
		return false;
	}

	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, typename TRange::iterator const& iter) {
		if (iter != ranges::end(range)) {
			if (std::addressof(*iter) != std::addressof(range.back())) std::swap(*iter, range.back());
			range.pop_back();
			return true;
		}
		return false;
	}

	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, typename TRange::value_type const& value) {
		return RemoveSwap(range, ranges::find(range, value));
	}

	template<typename TRange>
	bool RemoveSwap(TRange& range, std::span<typename TRange::value_type const> values) {
		bool removed = false;
		for (auto const& value : values) removed |= RemoveSwap(range, value);
		return removed;
	}

	/** Remove an element from  a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange, typename TPredicate>
	bool RemoveSwapIf(TRange& range, TPredicate&& predicate) {
		return RemoveSwap(range, ranges::find_if(range, std::forward<TPredicate>(predicate)));
	}

	template<typename TRange, typename TPredicate>
	size_t RemoveAllSwapIf(TRange& range, TPredicate&& predicate) {
		size_t numRemoved = 0;
		if (ranges::size(range) > 0) {
			//Handle from 0 to Size-2
			auto last = std::prev(ranges::end(range));
			for (auto iter = ranges::begin(range); iter != last;) {
				if (predicate(*iter)) {
					std::swap(*iter, *last);
					range.pop_back();
					last = ranges::prev(std::ranges::end(range));
					++numRemoved;
				}
				else {
					iter = ranges::next(iter);
				}
			}
			//Handle Size-1
			if (predicate(*last)) {
				range.pop_back();
				++numRemoved;
			}
		}
		return numRemoved;
	}

	/** Returns the index of the value in the range, or INVALID_INDEX if the value was not found */
	template<typename TRange>
	size_t IndexOf(TRange& range, typename TRange::ElementType const& value) {
		const auto iter = ranges::find(range, value);
		if (iter != ranges::end(range)) return std::distance(ranges::begin(range), iter);
		else return INVALID_INDEX;
	}

	/** Find the first element in the range that is equal to the value, and return a pointer to it. */
	template<typename TRange, typename TValue>
	typename TRange::value_type* Find(TRange& range, TValue const& value) {
		const auto iter = ranges::find(range, value);
		if (iter != ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}
	/** Find the first element in the range that is equal to the value, and return a pointer to it. */
	template<typename TRange, typename TValue>
	typename TRange::value_type const* Find(TRange const& range, TValue const& value) {
		const auto iter = ranges::find(range, value);
		if (iter != ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}

	/** Find the first element in the range for which the predicate returns true, and return a pointer to it. */
	template<typename TRange, typename TPredicate>
	typename TRange::value_type* FindIf(TRange& range, TPredicate&& predicate) {
		const auto iter = ranges::find_if(range, predicate);
		if (iter != ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}
	/** Find the first element in the range for which the predicate returns true, and return a pointer to it. */
	template<typename TRange, typename TPredicate>
	typename TRange::value_type const* FindIf(TRange const& range, TPredicate&& predicate) {
		const auto iter = ranges::find_if(range, predicate);
		if (iter != ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}
}
