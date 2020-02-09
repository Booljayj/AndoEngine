#pragma once
#include <cstddef>

template<typename T>
struct TArrayView {
	constexpr TArrayView() = default;

	//Create from a single value
	constexpr TArrayView(T const& value)
	: begin_(&value), size_(1)
	{}

	//Create from an iterable container
	template<typename ArrayType>
	constexpr TArrayView(ArrayType const& array)
	: begin_(array.data()), size_(array.end() - array.begin())
	{}

	constexpr inline T const& operator[](size_t index) const { return begin_[index]; }
	constexpr inline operator bool() const {return size_ == 0;}

	constexpr inline T const* begin() const { return begin_; }
	constexpr inline T const* end() const { return begin_ + size_; }
	constexpr inline size_t size() const { return size_; }

private:
	T const* begin_ = nullptr;
	size_t size_ = 0;
};
