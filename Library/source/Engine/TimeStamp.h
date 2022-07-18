#pragma once
#include "Engine/Bitfields.h"
#include "Engine/STL.h"

/** A broken-down time point used for displaying calendar information. Value ranges are set up for common usage, not for extremes. TimeStamps are not meant to be manually edited. */
union TimeStamp {
private:
	static constexpr uint8_t YearOffset = 0;
	static constexpr uint8_t YearNumBits = 14;
	static_assert((1ULL<<YearNumBits)-1 >= 9999, "Number of bits cannot contain required value range");

	static constexpr uint8_t MonthOffset = YearOffset + YearNumBits;
	static constexpr uint8_t MonthNumBits = 4;
	static_assert((1ULL<<MonthNumBits)-1 >= 12, "Number of bits cannot contain required value range");

	static constexpr uint8_t DayOffset = MonthOffset + MonthNumBits;
	static constexpr uint8_t DayNumBits = 5;
	static_assert((1ULL<<DayNumBits)-1 >= 31, "Number of bits cannot contain required value range");

	static constexpr uint8_t HourOffset = DayOffset + DayNumBits;
	static constexpr uint8_t HourNumBits = 5;
	static_assert((1ULL<<HourNumBits)-1 >= 23, "Number of bits cannot contain required value range");

	static constexpr uint8_t MinuteOffset = HourOffset + HourNumBits;
	static constexpr uint8_t MinuteNumBits = 6;
	static_assert((1ULL<<MinuteNumBits)-1 >= 59, "Number of bits cannot contain required value range");

	static constexpr uint8_t SecondOffset = MinuteOffset + MinuteNumBits;
	static constexpr uint8_t SecondNumBits = 6;
	static_assert((1ULL<<SecondNumBits)-1 >= 60, "Number of bits cannot contain required value range");

	static constexpr uint8_t MillisecondOffset = SecondOffset + SecondNumBits;
	static constexpr uint8_t MillisecondNumBits = 10;
	static_assert((1ULL<<MillisecondNumBits)-1 >= 999, "Number of bits cannot contain required value range");

public:
	using StorageType = uint64_t;

	StorageType value;

	// Bit layout for a timestamp is as follows:
	//  7        6        5        4        3        2        1        0
	//  ........-......pp-pppppppp-ssssssmm-mmmmhhhh-hDDDDDMM-MMYYYYYY-YYYYYYYY
	//  |63            |49         |39   |33    |27   |22  |17  |13           |0

	/** [0-9999] the gregorian calendar year */
	TBitfieldMember<StorageType, YearOffset, YearNumBits> year;
	/** [0-11] the month index */
	TBitfieldMember<StorageType, MonthOffset, MonthNumBits> month;
	/** [0-30] the day index in the current month */
	TBitfieldMember<StorageType, DayOffset, DayNumBits> day;

	/** [0-23] the hour of the day */
	TBitfieldMember<StorageType, HourOffset, HourNumBits> hour;
	/** [0-59] the minute of the hour */
	TBitfieldMember<StorageType, MinuteOffset, MinuteNumBits> minute;
	/** [0-60] the second of the minute (60 is used for leap seconds in some cases) */
	TBitfieldMember<StorageType, SecondOffset, SecondNumBits> second;
	/** [0-999] the milliseconds past the second */
	TBitfieldMember<StorageType, MillisecondOffset, MillisecondNumBits> millisecond;

	static TimeStamp Now() noexcept;

	TimeStamp() noexcept;
	TimeStamp(uint64_t inValue) noexcept;
	TimeStamp(std::chrono::system_clock::time_point const& timePoint) noexcept;

	/** Write this timestamp to the stream in the form "YYYY-MM-DD HH:MM:SS" */
	void Write(std::ostream& stream) const;
	/** Write this timestamp to the stream in the form "YYYY-MM-DD HH:MM:SS.sss" */
	void WritePrecise(std::ostream& stream) const;
};

static_assert(sizeof(TimeStamp)==sizeof(TimeStamp::StorageType), "Invalid TimeStamp union packing, TimeStamp must be exactly equal in size to the internal storage type");

std::ostream& operator<<(std::ostream&, TimeStamp const&);
