#pragma once
#include <cstddef>

template<typename T>
struct TArrayView {
	constexpr TArrayView() = default;

	template<typename TArray>
	constexpr TArrayView(const TArray& Array)
	: Begin(Array.begin()), Size(Array.size())
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
