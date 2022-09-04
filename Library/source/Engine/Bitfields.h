#pragma once
#include "Engine/StandardTypes.h"

template<typename StorageType_, uint8_t Offset_, uint8_t NumBits_>
struct TBitfieldMember {
	using StorageType = StorageType_;
	static constexpr uint8_t Offset = Offset_;
	static constexpr uint8_t NumBits = NumBits_;
	static constexpr StorageType Maximum = StorageType((1ULL << NumBits)-1);
	static constexpr StorageType Mask = (Maximum << Offset);

	static_assert(std::is_integral<StorageType_>::value && std::is_unsigned<StorageType_>::value, "StorageType must be an unsigned integer type");
	static_assert(NumBits > 0, "Bits cannot be 0");
	static_assert(NumBits < (sizeof(StorageType)*8), "Cannot fill an entire bitfield with one member");
	static_assert((Offset+NumBits) <= (sizeof(StorageType)*8), "Member exceeds bitfield boundaries");

	TBitfieldMember() = default;

	operator StorageType() const { return (memberValue >> Offset) & Maximum; }
	TBitfieldMember& operator=(StorageType V) { memberValue = (memberValue & ~Mask) | ((V & Maximum) << Offset); return *this; }

	StorageType Get() const { return (StorageType)(*this); }

private:
	StorageType memberValue;
};
