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
}
