#pragma once
#include "Engine/StandardTypes.h"

template<typename T>
struct TArrayView {
	constexpr TArrayView() = default;

	constexpr TArrayView(T const* inBegin, size_t inSize) : begin_(inBegin), size_(inSize) {}
	constexpr TArrayView(T const& value) : TArrayView(&value, 1) {}
	constexpr TArrayView(std::initializer_list<T> const& list) : TArrayView(list.begin(), list.size()) {}

	template<size_t Size> constexpr TArrayView(T const (&array)[Size]) : TArrayView(array, Size) {}
	template<size_t Size> constexpr TArrayView(std::array<std::remove_const_t<T>, Size> const& array) : TArrayView(array.data(), Size) {}
	template<typename Allocator> constexpr TArrayView(std::vector<std::remove_const_t<T>, Allocator> const& vector) : TArrayView(vector.data(), vector.size()) {}

	constexpr inline T const& operator[](size_t index) const { return begin_[index]; }
	constexpr inline operator bool() const {return size_ == 0;}

	constexpr inline T const* begin() const { return begin_; }
	constexpr inline T const* end() const { return begin_ + size_; }
	constexpr inline size_t size() const { return size_; }

	constexpr inline size_t IndexOf(const T& value) const {
		auto const iter = std::find(begin(), end(), value);
		return iter - begin();
	}
	template<typename PredicateType>
	constexpr inline size_t IndexOf(const PredicateType& predicate) const {
		auto const iter = std::find_if(begin(), end(), predicate);
		return iter - begin();
	}

	constexpr inline T const* Find(const T& value) const {
		auto const iter = std::find(begin(), end(), value);
		if (iter != end()) return iter;
		else return nullptr;
	}
	template<typename PredicateType>
	constexpr inline T const* Find(const PredicateType& predicate) const {
		auto const iter = std::find_if(begin(), end(), predicate);
		if (iter != end()) return iter;
		else return nullptr;
	}

private:
	T const* begin_ = nullptr;
	size_t size_ = 0;
};
