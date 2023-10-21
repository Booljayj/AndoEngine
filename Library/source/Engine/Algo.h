#include "Engine/StandardTypes.h"

namespace Algo {
	constexpr size_t InvalidIndex = std::numeric_limits<size_t>::max();

	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, size_t index) {
		const size_t size = std::ranges::size(range);
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
		if (iter != std::ranges::end(range)) {
			if (std::addressof(*iter) != std::addressof(range.back())) std::swap(*iter, range.back());
			range.pop_back();
			return true;
		}
		return false;
	}

	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, typename TRange::value_type const& value) {
		return RemoveSwap(range, std::ranges::find(range, value));
	}

	/** Remove an element from  a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange, typename TPredicate>
	bool RemoveSwapIf(TRange& range, TPredicate&& predicate) {
		return RemoveSwap(range, std::ranges::find_if(range, std::forward<TPredicate>(predicate)));
	}

	template<typename TRange, typename TPredicate>
	size_t RemoveAllSwapIf(TRange& range, TPredicate&& predicate) {
		size_t numRemoved = 0;
		if (std::ranges::size(range) > 0) {
			//Handle from 0 to Size-2
			auto last = std::prev(std::ranges::end(range));
			for (auto iter = std::ranges::begin(range); iter != last;) {
				if (predicate(*iter)) {
					std::swap(*iter, *last);
					range.pop_back();
					last = std::ranges::prev(std::ranges::end(range));
					++numRemoved;
				} else {
					iter = std::ranges::next(iter);
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

	/** Returns the index of the value in the range, or InvalidIndex if the value was not found */
	template<typename TRange>
	size_t IndexOf(TRange& range, typename TRange::ElementType const& value) {
		const auto iter = std::ranges::find(range, value);
		if (iter != std::ranges::end(range)) return std::distance(std::ranges::begin(range), iter);
		else return InvalidIndex;
	}

	/** Find the first element in the range that is equal to the value, and return a pointer to it. */
	template<typename TRange, typename TValue>
	typename TRange::value_type* Find(TRange& range, TValue const& value) {
		const auto iter = std::ranges::find(range, value);
		if (iter != std::ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}
	/** Find the first element in the range that is equal to the value, and return a pointer to it. */
	template<typename TRange, typename TValue>
	typename TRange::value_type const* Find(TRange const& range, TValue const& value) {
		const auto iter = std::ranges::find(range, value);
		if (iter != std::ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}

	/** Find the first element in the range for which the predicate returns true, and return a pointer to it. */
	template<typename TRange, typename TPredicate>
	typename TRange::value_type* FindIf(TRange& range, TPredicate&& predicate) {
		const auto iter = std::ranges::find_if(range, predicate);
		if (iter != std::ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}
	/** Find the first element in the range for which the predicate returns true, and return a pointer to it. */
	template<typename TRange, typename TPredicate>
	typename TRange::value_type const* FindIf(TRange const& range, TPredicate&& predicate) {
		const auto iter = std::ranges::find_if(range, predicate);
		if (iter != std::ranges::end(range)) return std::addressof(*iter);
		else return nullptr;
	}
}
