#include "Engine/StandardTypes.h"

namespace Algo {
	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, size_t index) {
		if (index < range.size()) {
			if (index < (range.size() - 1)) std::swap(range[index], range.back());
			range.pop_back();
			return true;
		}
		return false;
	}

	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, typename TRange::iterator const& iter) {
		if (iter != range.end()) {
			if (std::addressof(*iter) != std::addressof(range.back())) std::swap(*iter, range.back());
			range.pop_back();
			return true;
		}
		return false;
	}

	/** Remove an element from a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange>
	bool RemoveSwap(TRange& range, typename TRange::value_type const& value) {
		return RemoveSwap(range, std::find(range.begin(), range.end(), value));
	}

	/** Remove an element from  a container by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange, typename TPredicate>
	bool RemoveSwapIf(TRange& range, TPredicate&& predicate) {
		return RemoveSwap(range, std::find_if(range.begin(), range.end(), std::forward<TPredicate>(predicate)));
	}

	template<typename TRange>
	size_t IndexOf(TRange& range, typename TRange::ElementType const& value) {
	}

	/** Find the first element in the range that is equal to the value, and return a pointer to it. */
	template<typename TRange, typename TValue>
	typename TRange::value_type* Find(TRange& range, TValue const& value) {
		const auto iter = std::find(range.begin(), range.end(), value);
		if (iter != range.end()) return std::addressof(*iter);
		else return nullptr;
	}
	/** Find the first element in the range that is equal to the value, and return a pointer to it. */
	template<typename TRange, typename TValue>
	typename TRange::value_type const* Find(TRange const& range, TValue const& value) {
		const auto iter = std::find(range.cbegin(), range.cend(), value);
		if (iter != range.cend()) return std::addressof(*iter);
		else return nullptr;
	}

	/** Find the first element in the range for which the predicate returns true, and return a pointer to it. */
	template<typename TRange, typename TPredicate>
	typename TRange::value_type* FindIf(TRange& range, TPredicate&& predicate) {
		const auto iter = std::find_if(range.begin(), range.end(), predicate);
		if (iter != range.end()) return std::addressof(*iter);
		else return nullptr;
	}
	/** Find the first element in the range for which the predicate returns true, and return a pointer to it. */
	template<typename TRange, typename TPredicate>
	typename TRange::value_type const* FindIf(TRange const& range, TPredicate&& predicate) {
		const auto iter = std::find_if(range.cbegin(), range.cend(), predicate);
		if (iter != range.cend()) return std::addressof(*iter);
		else return nullptr;
	}
}
