#pragma once

namespace Rendering {
	template<typename ValueType, typename EnumType>
	struct EnumBackedContainer {
	public:
		EnumBackedContainer() { std::memset(&array, 0, sizeof(array)); }

		ValueType& operator[](EnumType value) { return array[static_cast<std::underlying_type_t<EnumType>>(value)]; }
		ValueType const& operator[](EnumType value) const { return array[static_cast<std::underlying_type_t<EnumType>>(value)]; }

		uint32_t size() const { return static_cast<uint32_t>(EnumType::MAX); }
		ValueType const* data() const { return array.data(); }

	private:
		std::array<ValueType, static_cast<uint8_t>(EnumType::MAX)> array;
	};
}
