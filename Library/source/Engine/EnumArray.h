#pragma once
#include "Engine/Array.h"
#include "Engine/Concepts.h"
#include "Engine/Core.h"

/** An array that is indexed with a enum class type, typically with a size that matches the number of elements in the enum */
template<typename ValueType, Concepts::Enumeration EnumType, size_t Size = static_cast<size_t>(EnumType::MAX)>
struct EnumArray {
public:
	EnumArray() { array.fill(ValueType{}); }
	EnumArray(EnumType value) { array.fill(static_cast<EnumType>(value)); }

	ValueType& operator[](EnumType value) { return array[std::to_underlying(value)]; }
	ValueType const& operator[](EnumType value) const { return array[std::to_underlying(value)]; }

	uint32_t size() const { return static_cast<uint32_t>(EnumType::MAX); }
	ValueType const* data() const { return array.data(); }

private:
	std::array<ValueType, Size> array;
};

template<typename ValueType, Concepts::Enumeration EnumType, size_t Size>
inline std::span<ValueType const, Size> MakeSpan(EnumArray<ValueType, EnumType, Size> const& enum_array) {
	return std::span<ValueType const, Size>{ enum_array.data(), enum_array.size() };
}