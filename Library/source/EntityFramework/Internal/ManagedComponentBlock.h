#pragma once
#include <array>
#include <bitset>
#include <cassert>

template<typename ComponentType, size_t BlockSize>
struct TInlineManagedComponentBlock {
	size_t lowestFreeIndex;
	std::bitset<BlockSize> used;
	std::array<ComponentType, BlockSize> data;

	TInlineManagedComponentBlock()
	: lowestFreeIndex(0)
	{}

	size_t CountUsed() const { return used.count(); }
	bool Contains(ComponentType const* value) const { return value >= data.begin() && value < data.end(); }
	bool HasAnyFree() const { return lowestFreeIndex < BlockSize; }
	size_t IndexOf(ComponentType const* value) const { return (value - data.begin()); }
	bool IsUsed(ComponentType const* value) const { return used[IndexOf(value)]; }

	ComponentType* Retain() {
		assert(HasAnyFree());
		ComponentType* retained = &data[lowestFreeIndex];
		used.set(lowestFreeIndex);
		do {
			++lowestFreeIndex;
		} while (lowestFreeIndex < BlockSize && used.test(lowestFreeIndex));

		return retained;
	}

	void Release(ComponentType* released) {
		assert(Contains(released));
		size_t releasedIndex = IndexOf(released);
		used.reset(releasedIndex);
		if (releasedIndex < lowestFreeIndex) lowestFreeIndex = releasedIndex;
	}

	size_t size() const { return data.size(); }
	typename std::array<ComponentType, BlockSize>::const_iterator begin() const { return data.begin(); }
	typename std::array<ComponentType, BlockSize>::const_iterator end() const { return data.end(); }
	typename std::array<ComponentType, BlockSize>::iterator begin() { return data.begin(); }
	typename std::array<ComponentType, BlockSize>::iterator end() { return data.end(); }
};

template<typename ComponentType>
struct THeapManagedComponentBlock {
	//Wrapper so ranged-based for loops will work with pointers
	struct Iterator {
		Iterator(ComponentType* inPtr) : ptr(inPtr) {}
		void operator++() { ++ptr; }
		bool operator!=(Iterator const& other) const { return ptr != other.ptr; }
		ComponentType* operator*() const { return ptr; }

	private:
		ComponentType* ptr;
	};

	ComponentType* data = nullptr;
	ComponentType* capacity = nullptr;
	ComponentType* lowestFree = nullptr;
	uint8_t* usedBytes = nullptr;
	size_t usedCount = 0;

	THeapManagedComponentBlock(size_t blockSize) {
		assert(blockSize > 0);
		data = static_cast<ComponentType*>(std::malloc(sizeof(ComponentType) * blockSize));
		capacity = data + blockSize;
		lowestFree = data;

		//You would normally add one only to catch the remainder, but why not just do it every time?
		size_t numUsageTrackingBytes = (blockSize / 8) + 1;
		usedBytes = static_cast<uint8_t*>(std::malloc(numUsageTrackingBytes));
		assert(data != nullptr);
	}

	~THeapManagedComponentBlock() {
		std::free(static_cast<void*>(data));
		std::free(static_cast<void*>(usedBytes));
	}

	size_t CountUsed() const { return usedCount; }
	bool Contains(ComponentType const* value) const { return value >= data && value < capacity; }
	bool HasAnyFree() const { return lowestFree < capacity; }
	size_t IndexOf(ComponentType const* value) const { return value - data; }
	bool IsUsed(ComponentType const* value) const {
		const size_t index = IndexOf(value);
		return TEST_BIT(*(usedBytes + (index / 8)), (1 << (index / 8)));
	}
	void SetUsed(ComponentType const* value) {
		++usedCount;
		const size_t index = IndexOf(value);
		SET_BIT(*(usedBytes + (index / 8)), (1 << (index / 8)));
	}
	void SetFree(ComponentType const* value) {
		--usedCount;
		const size_t index = IndexOf(value);
		CLEAR_BIT(*(usedBytes + (index / 8)), (1 << (index / 8)));
	}

	ComponentType* Retain() {
		assert(HasAnyFree());
		ComponentType* retained = lowestFree;
		SetUsed(retained);
		while (lowestFree < capacity && IsUsed(lowestFree)) {
			++lowestFree;
		}
		return retained;
	}

	void Release(ComponentType* released) {
		assert(Contains(released));
		SetFree(released);
		if (released < lowestFree) {
			lowestFree = released;
		}
	}

	Iterator begin() const { return Iterator{data}; }
	Iterator end() const { return Iterator{capacity}; }
};
