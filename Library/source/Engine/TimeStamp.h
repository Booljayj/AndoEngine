#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/Utility.h"

/** A broken-down time point used for displaying year-month-day calendar information */
struct CalendarTimeStamp {
	using StorageType = uint32_t;

	using TimePointType = std::chrono::system_clock::time_point;
	using DaysType = std::chrono::duration<int64_t, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>>;
	using YearCountType = uint16_t;
	using MonthCountType = uint8_t;
	using DayCountType = uint8_t;

	static constexpr uint16_t YearMax = 9999; //Years are always CE, and we only support up to four year digits.
	static constexpr uint16_t MonthMax = 12;
	static constexpr uint16_t DayMax = 31;

	/** [0-9999] the gregorian calendar year */
	StorageType year : Utility::GetMinimumNumBits(YearMax) = 0;
	/** [0-11] the month index */
	StorageType month : Utility::GetMinimumNumBits(MonthMax) = 0;
	/** [0-30] the day index in the current month */
	StorageType day : Utility::GetMinimumNumBits(DayMax) = 0;

	static CalendarTimeStamp Now();

	CalendarTimeStamp() = default;
	CalendarTimeStamp(YearCountType year, MonthCountType month, DayCountType day) noexcept : year(year), month(month), day(day) {}
	CalendarTimeStamp(DaysType const& daysSinceEpochStart) noexcept;
	CalendarTimeStamp(TimePointType const& timepoint) noexcept : CalendarTimeStamp(GetDaysSinceEpoch(timepoint)) {}

private:
	static_assert(sizeof(YearCountType) * CHAR_BIT >= Utility::GetMinimumNumBits(YearMax));
	static_assert(sizeof(MonthCountType) * CHAR_BIT >= Utility::GetMinimumNumBits(MonthMax));
	static_assert(sizeof(DayCountType) * CHAR_BIT >= Utility::GetMinimumNumBits(DayMax));

	DaysType GetDaysSinceEpoch(TimePointType const& timepoint);
};
static_assert(sizeof(CalendarTimeStamp) == sizeof(CalendarTimeStamp::StorageType), "CalendarTimeStamp must be tightly packed");

/** A broken-down time point used for displaying hour-minute-second clock information. */
struct ClockTimeStamp {
	using StorageType = uint32_t;

	using TimePointType = std::chrono::system_clock::time_point;
	using MillisecondsType = std::chrono::milliseconds;
	using HourCountType = uint8_t;
	using MinuteCountType = uint8_t;
	using SecondCountType = uint8_t;
	using MillisecondCountType = uint16_t;

	static constexpr uint16_t HourMax = 23; //Hours are 0-indexed internally, where 0 is 12am.
	static constexpr uint16_t MinuteMax = 59;
	static constexpr uint16_t SecondMax = 60; //Full 60 seconds is required to support leap seconds
	static constexpr uint16_t MillisecondMax = 999;

	/** [0-23] the hour of the day */
	StorageType hour : Utility::GetMinimumNumBits(HourMax) = 0;
	/** [0-59] the minute of the hour */
	StorageType minute : Utility::GetMinimumNumBits(MinuteMax) = 0;
	/** [0-60] the second of the minute (60 is used for leap seconds in some cases) */
	StorageType second : Utility::GetMinimumNumBits(SecondMax) = 0;
	/** [0-999] the milliseconds past the second */
	StorageType millisecond : Utility::GetMinimumNumBits(MillisecondMax) = 0;

	static ClockTimeStamp Now();

	ClockTimeStamp() = default;
	ClockTimeStamp(HourCountType hour, MinuteCountType minute, SecondCountType second, MillisecondCountType millisecond) noexcept : hour(hour), minute(minute), second(second), millisecond(millisecond) {}
	ClockTimeStamp(MillisecondsType const& millisecondsSinceDayStart) noexcept;
	ClockTimeStamp(TimePointType const& timepoint) noexcept : ClockTimeStamp(GetMillisecondsSinceDayStart(timepoint)) {}

private:
	static_assert(sizeof(HourCountType) * CHAR_BIT >= Utility::GetMinimumNumBits(HourMax));
	static_assert(sizeof(MinuteCountType) * CHAR_BIT >= Utility::GetMinimumNumBits(MinuteMax));
	static_assert(sizeof(SecondCountType) * CHAR_BIT >= Utility::GetMinimumNumBits(HourMax));
	static_assert(sizeof(MillisecondCountType) * CHAR_BIT >= Utility::GetMinimumNumBits(HourMax));

	static MillisecondsType GetMillisecondsSinceDayStart(TimePointType const& timepoint);
};
static_assert(sizeof(ClockTimeStamp) == sizeof(ClockTimeStamp::StorageType), "ClockTimeStamp must be tightly packed");

struct TimeStamp {
	using TimePointType = std::chrono::system_clock::time_point;

	CalendarTimeStamp calendar;
	ClockTimeStamp clock;

	static TimeStamp Now();

	TimeStamp() = default;
	TimeStamp(TimePointType const& timepoint);
};

template<>
struct std::formatter<CalendarTimeStamp> : std::formatter<std::string_view> {
	auto format(const CalendarTimeStamp& calendar, format_context& ctx) const {
		char scratch[11] = { 0 };
		auto const result = std::format_to_n(scratch, sizeof(scratch), "{:0>4}-{:0>2}-{:0>2}", calendar.year, calendar.month + 1, calendar.day + 1);
		return formatter<string_view>::format(string_view{ scratch, result.out }, ctx);
	}
};

template<>
struct std::formatter<ClockTimeStamp> : std::formatter<std::string_view> {
	auto format(const ClockTimeStamp& clock, format_context& ctx) const {
		char scratch[13] = { 0 };
		auto const result = std::format_to_n(scratch, sizeof(scratch), "{:0>2}:{:0>2}:{:0>2}.{:0>3}", clock.hour, clock.minute, clock.second, clock.millisecond);
		return formatter<string_view>::format(string_view{ scratch, result.out }, ctx);
	}
};

template<>
struct std::formatter<TimeStamp> : std::formatter<std::string_view> {
	auto format(const TimeStamp& time, format_context& ctx) const {
		char scratch[25] = { 0 };
		auto const result = std::format_to_n(scratch, sizeof(scratch), "{} {}", time.calendar, time.clock);
		return formatter<string_view>::format(string_view{ scratch, result.out }, ctx);
	}
};
