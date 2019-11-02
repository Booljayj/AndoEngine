#pragma once
#include <cstddef>

template<typename T>
struct TArrayView {
	constexpr TArrayView() = default;

	//Create from a single value
	constexpr TArrayView(T const& Value)
	: Begin(&Value), Size(1)
	{}

	//Create from an iterable container
	template<typename ArrayType>
	constexpr TArrayView(ArrayType const& Array)
	: Begin(Array.begin()), Size(Array.end() - Array.begin())
	{}

	constexpr inline T const& operator[](size_t Index) const { return Begin[Index]; }
	constexpr inline operator bool() const {return Size == 0;}

	constexpr inline T const* begin() const { return Begin; }
	constexpr inline T const* end() const { return Begin + Size; }
	constexpr inline size_t size() const { return Size; }

private:
	T const* Begin = nullptr;
	size_t Size = 0;
};
