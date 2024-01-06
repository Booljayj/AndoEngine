#include "Engine/TimeStamp.h"
#include "Engine/Utility.h"

CalendarTimeStamp CalendarTimeStamp::Now() {
	return CalendarTimeStamp{ std::chrono::system_clock::now() };
}

CalendarTimeStamp::CalendarTimeStamp(DaysType const& daysSinceEpochStart) noexcept {
	// The shift of days to move the epoch from 1970-01-01 to 0000-03-01
	constexpr int64_t EpochShift = 719468;
	// The number of days in an era, a 400-year period where dates will repeat
	constexpr int64_t DaysPerEra = 146097;
	// The number of days in a non-leap year
	constexpr uint32_t DaysPerYear = 365;

	//The Slope used in the linear equation that relates month numbers to the day of the year when they start
	constexpr uint32_t MonthSlope = 153;
	//The Offset used in the linear equation that relates month numbers to the day of the year when they start
	constexpr uint32_t MonthOffset = 2;
	//The Scale used in the linear equation that relates month numbers to the day of the year when they start
	constexpr uint32_t MonthScale = 5;

	//Many of these calculations have to handle leap days. For clarity:
	//- Leap days occur every 4 years, unless the year is divisible by 100, unless the year is also divisible by 400.

	int64_t const daysSinceEpoch = daysSinceEpochStart.count() + EpochShift;
	//Calculate the era, a 400-year period also known as the Leap Cycle, where dates will repeat exactly.
	int64_t const era = (daysSinceEpoch >= 0 ? daysSinceEpoch : daysSinceEpoch - (DaysPerEra - 1)) / DaysPerEra;
	//Calculate the number of days since the start of the era
	uint32_t const dayOfEra = static_cast<uint32_t>(daysSinceEpoch - (era * DaysPerEra)); //[0-146,096]
	//Calculate the year of the era correcting for leap days, which occur every 4 years except for years divisible by 100 unless they are also divisible by 400.
	uint32_t const yearOfEra = (dayOfEra - dayOfEra/1460 + dayOfEra/36524 - dayOfEra/146096) / DaysPerYear; //[0-399]

	//Calculate the day of the year correcting for leap years.
	uint32_t const dayOfYear = dayOfEra - (DaysPerYear*yearOfEra + yearOfEra/4 - yearOfEra/100); //[0-365]
	//Calculate the month of the year using a linear equation. Note that this assumes May is the first month of the year and has index 0.
	uint32_t const monthPrime = (MonthScale*dayOfYear + MonthOffset)/MonthSlope; //[0-11]
	//Calculate the high-precision year that will be clamped down later. Note that this year assumes January is the first month of the year.
	int64_t const highPrecisionYear = static_cast<int64_t>(yearOfEra) + (era * 400) + (monthPrime >= 10);

	//Calculate the day of the month using our linear equation that relates to the starting day of each month
	day = dayOfYear - (MonthSlope*monthPrime + MonthOffset)/MonthScale; //[0-30]
	//Convert from our May-centered month number to a January-centered month
	month = monthPrime + (monthPrime < 10 ? 3 : -9) - 1; //[0-11]
	//Clamp the year down to the range we care about for TimeStamps
	//@todo std::clamp is not available until C++17. Replace this when that is available.
	year = static_cast<uint16_t>(std::max<int64_t>(0, std::min<int64_t>(9999, highPrecisionYear))); //[0-9999]
}

CalendarTimeStamp::DaysType CalendarTimeStamp::GetDaysSinceEpoch(TimePointType const& timepoint) {
	return std::chrono::duration_cast<DaysType>(timepoint.time_since_epoch());
}

ClockTimeStamp ClockTimeStamp::Now() {
	return ClockTimeStamp{ std::chrono::system_clock::now() };
}

ClockTimeStamp::ClockTimeStamp(MillisecondsType const& millisecondsSinceDayStart) noexcept {
	constexpr uint32_t MillisecondsPerMinute = 1000 * 60;
	constexpr uint32_t MillisecondsPerHour = MillisecondsPerMinute * 60;

	uint32_t ms = static_cast<uint32_t>(millisecondsSinceDayStart.count());

	hour = ms / MillisecondsPerHour; //[0-23]
	ms %= MillisecondsPerHour;

	minute = ms / MillisecondsPerMinute; //[0-59]
	ms %= MillisecondsPerMinute;

	second = ms / 1000; //[0-59]
	ms %= 1000;

	millisecond = ms; //[0-999]
}

ClockTimeStamp::MillisecondsType ClockTimeStamp::GetMillisecondsSinceDayStart(TimePointType const& timepoint) {
	auto const nowUTC = timepoint.time_since_epoch();
	auto const todayUTC = std::chrono::duration_cast<CalendarTimeStamp::DaysType>(nowUTC);
	auto const duration = nowUTC - todayUTC;
	return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
}

TimeStamp TimeStamp::Now() {
	return TimeStamp{ std::chrono::system_clock::now() };
}

TimeStamp::TimeStamp(TimePointType const& timepoint) {
	auto const timeSinceEpoch = timepoint.time_since_epoch();
	auto const daysSinceEpochStart = std::chrono::duration_cast<CalendarTimeStamp::DaysType>(timeSinceEpoch);
	auto const millisecondsSinceDayStart = std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceEpoch - daysSinceEpochStart);

	calendar = CalendarTimeStamp{ daysSinceEpochStart };
	clock = ClockTimeStamp{ millisecondsSinceDayStart };
}
