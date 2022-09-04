#include "Engine/StandardTypes.h"

namespace Algo {
	/** Remove an element from the array by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange, typename TRangeValue>
	bool RemoveSwap(TRange& range, TRangeValue const& value) {
		auto const iter = std::find(range.begin(), range.end(), value);
		if (iter != range.end()) {
			std::iter_swap(iter, range.end() - 1);
			range.pop_back();
			return true;
		}
		else return false;
	}

	/** Remove an element from the array by swapping it with the last element and removing the last. Faster removal but does not preserve order. */
	template<typename TRange, typename TPredicate>
	bool RemoveSwapIf(TRange& range, TPredicate&& predicate) {
		auto const iter = std::find_if(range.begin(), range.end(), std::forward<TPredicate>(predicate));
		if (iter != range.end()) {
			std::iter_swap(iter, range.end() - 1);
			range.pop_back();
			return true;
		}
		else return false;
	}
}
