#pragma once
#include "Engine/Bitfields.h"
#include "Engine/StandardTypes.h"

/** A broken-down time point used for displaying year-month-day calendar information */
union CalendarTimeStamp {
private:
	static constexpr uint16_t YearMax = 9999; //Years are always CE, and we only support up to four year digits.
	static constexpr uint8_t YearOffset = 0;
	static constexpr uint8_t YearNumBits = 14;
	static_assert((1ULL<<YearNumBits)-1 >= YearMax, "Number of bits cannot contain required value range");

	static constexpr uint16_t MonthMax = 12; //Months are 1-indexed
	static constexpr uint8_t MonthOffset = YearOffset + YearNumBits;
	static constexpr uint8_t MonthNumBits = 4;
	static_assert((1ULL<<MonthNumBits)-1 >= 12, "Number of bits cannot contain required value range");

	static constexpr uint16_t DayMax = 31; //Days are 1-indexed
	static constexpr uint8_t DayOffset = MonthOffset + MonthNumBits;
	static constexpr uint8_t DayNumBits = 5;
	static_assert((1ULL<<DayNumBits)-1 >= 31, "Number of bits cannot contain required value range");

public:
	using TimePointType = std::chrono::system_clock::time_point;
	using DaysType = std::chrono::duration<int64_t, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>>;
	using StorageType = uint32_t;

	StorageType value = (StorageType)0;

	/** [0-9999] the gregorian calendar year */
	TBitfieldMember<StorageType, YearOffset, YearNumBits> year;
	/** [0-11] the month index */
	TBitfieldMember<StorageType, MonthOffset, MonthNumBits> month;
	/** [0-30] the day index in the current month */
	TBitfieldMember<StorageType, DayOffset, DayNumBits> day;

	static CalendarTimeStamp Now();

	CalendarTimeStamp() = default;
	CalendarTimeStamp(uint64_t inValue) noexcept : value(inValue) {}
	CalendarTimeStamp(DaysType const& daysSinceEpochStart) noexcept;
	CalendarTimeStamp(TimePointType const& timepoint) noexcept;

private:
	DaysType GetDaysFromTimePoint(TimePointType const& timepoint);
};
static_assert(sizeof(CalendarTimeStamp)==sizeof(CalendarTimeStamp::StorageType), "CalendarTimeStamp must be tightly packed");

/** A broken-down time point used for displaying hour-minute-second clock information. */
union ClockTimeStamp {
private:
	static constexpr uint16_t HourMax = 23; //Hours are 0-indexed internally, where 0 is 12am.
	static constexpr uint8_t HourOffset = 0;
	static constexpr uint8_t HourNumBits = 5;
	static_assert((1ULL<<HourNumBits)-1 >= HourMax, "Number of bits cannot contain required value range");

	static constexpr uint16_t MinuteMax = 59;
	static constexpr uint8_t MinuteOffset = HourOffset + HourNumBits;
	static constexpr uint8_t MinuteNumBits = 6;
	static_assert((1ULL<<MinuteNumBits)-1 >= MinuteMax, "Number of bits cannot contain required value range");

	static constexpr uint16_t SecondMax = 60; //Full 60 seconds is required for leap seconds support
	static constexpr uint8_t SecondOffset = MinuteOffset + MinuteNumBits;
	static constexpr uint8_t SecondNumBits = 6;
	static_assert((1ULL<<SecondNumBits)-1 >= SecondMax, "Number of bits cannot contain required value range");

	static constexpr uint16_t MillisecondMax = 999;
	static constexpr uint8_t MillisecondOffset = SecondOffset + SecondNumBits;
	static constexpr uint8_t MillisecondNumBits = 10;
	static_assert((1ULL<<MillisecondNumBits)-1 >= MillisecondMax, "Number of bits cannot contain required value range");

public:
	using TimePointType = std::chrono::system_clock::time_point;
	using MillisecondsType = std::chrono::milliseconds;
	using StorageType = uint32_t;

	StorageType value = (StorageType)0;

	/** [0-23] the hour of the day */
	TBitfieldMember<StorageType, HourOffset, HourNumBits> hour;
	/** [0-59] the minute of the hour */
	TBitfieldMember<StorageType, MinuteOffset, MinuteNumBits> minute;
	/** [0-60] the second of the minute (60 is used for leap seconds in some cases) */
	TBitfieldMember<StorageType, SecondOffset, SecondNumBits> second;
	/** [0-999] the milliseconds past the second */
	TBitfieldMember<StorageType, MillisecondOffset, MillisecondNumBits> millisecond;

	static ClockTimeStamp Now();

	ClockTimeStamp() = default;
	ClockTimeStamp(uint64_t inValue) noexcept : value(inValue) {}
	ClockTimeStamp(MillisecondsType const& millisecondsSinceDayStart) noexcept;
	ClockTimeStamp(TimePointType const& timepoint) noexcept;

private:
	static MillisecondsType GetMillisecondsFromTimePoint(TimePointType const& timepoint);
};
static_assert(sizeof(ClockTimeStamp)==sizeof(ClockTimeStamp::StorageType), "ClockTimeStamp must be tightly packed");

struct TimeStamp {
	using TimePointType = std::chrono::system_clock::time_point;

	CalendarTimeStamp calendar;
	ClockTimeStamp clock;

	static TimeStamp Now();

	TimeStamp() = default;
	TimeStamp(TimePointType const& timepoint);
};

std::ostream& operator<<(std::ostream& stream, CalendarTimeStamp const& calendar);
std::ostream& operator<<(std::ostream& stream, ClockTimeStamp const& clock);
std::ostream& operator<<(std::ostream& stream, TimeStamp const& timestamp);
