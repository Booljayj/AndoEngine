#include "Engine/TimeStamp.h"

void TimeStamp::WriteReversedValue( uint16_t Value, const uint8_t NumDigits, char* Buffer) noexcept {
	//Generate the digits in reverse order
	char const* const End = Buffer+NumDigits;
	for(; Value > 0 && Buffer < End; ++Buffer) {
		*Buffer = (Value % 10) + '0'; //Write the lowest digit
		Value /= 10; //Shift all digits down
	}
}

TimeStamp TimeStamp::Now() noexcept {
	return TimeStamp{ std::chrono::system_clock::now() };
}

TimeStamp::TimeStamp() noexcept
: Value(DefaultValue)
{}

TimeStamp::TimeStamp(uint64_t InValue) noexcept
: Value(InValue)
{}

TimeStamp::TimeStamp(std::chrono::system_clock::time_point const& TimePoint) noexcept
: TimeStamp()
{
	//This algorithm was created by Howard Hinnant.
	//Documented here: http://howardhinnant.github.io/date_algorithms.html
	//Implemented by the original author here: https://howardhinnant.github.io/date/date.html

	using days_t = std::chrono::duration<int64_t, std::ratio_multiply<std::chrono::hours::period, std::ratio<24>>>;

	auto const NowUTC = TimePoint.time_since_epoch();
	auto const TodayUTC = std::chrono::duration_cast<days_t>(NowUTC);

	// The shift of days to move the epoch from 1970-01-01 to 0000-03-01
	constexpr int64_t EPOCH_SHIFT = 719468;
	// The number of days in an era, a 400-year period where dates will repeat
	constexpr int64_t DAYS_PER_ERA = 146097;
	// The number of days in a non-leap year
	constexpr uint32_t DAYS_PER_YEAR = 365;

	//The Slope used in the linear equation that relates month numbers to the day of the year when they start
	constexpr uint32_t MONTH_SLOPE = 153;
	//The Offset used in the linear equation that relates month numbers to the day of the year when they start
	constexpr uint32_t MONTH_OFFSET = 2;
	//The Scale used in the linear equation that relates month numbers to the day of the year when they start
	constexpr uint32_t MONTH_SCALE = 5;

	//Many of these calculations have to handle leap days. For clarity:
	//- Leap days occur every 4 years, unless the year is divisible by 100, unless the year is also divisible by 400.

	int64_t const DaysSinceEpoch = TodayUTC.count() + EPOCH_SHIFT;
	//Calculate the era, a 400-year period also known as the Leap Cycle, where dates will repeat exactly.
	int64_t const Era = (DaysSinceEpoch >= 0 ? DaysSinceEpoch : DaysSinceEpoch - (DAYS_PER_ERA - 1)) / DAYS_PER_ERA;
	//Calculate the number of days since the start of the era
	uint32_t const DayOfEra = static_cast<uint32_t>(DaysSinceEpoch - (Era * DAYS_PER_ERA)); //[0-146,096]
	//Calculate the year of the era correcting for leap days, which occur every 4 years except for years divisible by 100 unless they are also divisible by 400.
	uint32_t const YearOfEra = (DayOfEra - DayOfEra/1460 + DayOfEra/36524 - DayOfEra/146096) / DAYS_PER_YEAR; //[0-399]

	//Calculate the day of the year correcting for leap years.
	uint32_t const DayOfYear = DayOfEra - (DAYS_PER_YEAR*YearOfEra + YearOfEra/4 - YearOfEra/100); //[0-365]
	//Calculate the month of the year using a linear equation. Note that this assumes May is the first month of the year and has index 0.
	uint32_t const MonthPrime = (MONTH_SCALE*DayOfYear + MONTH_OFFSET)/MONTH_SLOPE; //[0-11]
	//Calculate the high-precision year that will be clamped down later. Note that this year assumes January is the first month of the year.
	int64_t const HighPrecisionYear = static_cast<int64_t>(YearOfEra) + (Era * 400) + (MonthPrime >= 10);

	//Calculate the day of the month using our linear equation that relates to the starting day of each month
	Day = 1 + DayOfYear - (MONTH_SLOPE*MonthPrime + MONTH_OFFSET)/MONTH_SCALE; //[1-31]
	//Convert from our May-centered month number to a January-centered month
	Month = MonthPrime + (MonthPrime < 10 ? 3 : -9); //[1-12]
	//Clamp the year down to the range we care about for TimeStamps
	//@todo std::clamp is not available until C++17. Replace this when that is available.
	Year = static_cast<uint16_t>(
		HighPrecisionYear < 0 ?
			0 :
			HighPrecisionYear > 9999 ?
				9999 :
				HighPrecisionYear
	); //[0-9999]

	static constexpr uint32_t MillisecondsPerMinute = 1000 * 60;
	static constexpr uint32_t MillisecondsPerHour = MillisecondsPerMinute * 60;

	//Calculate the duration since this day started in milliseconds
	auto Duration = NowUTC - TodayUTC;
	uint32_t MS = static_cast<uint32_t>( std::chrono::duration_cast<std::chrono::milliseconds>( Duration ).count() );

	Hour = MS / MillisecondsPerHour; //[0-23]
	MS %= MillisecondsPerHour;

	Minute = MS / MillisecondsPerMinute; //[0-59]
	MS %= MillisecondsPerMinute;

	Second = MS / 1000; //[0-59]
	MS %= 1000;

	Millisecond = MS; //[0-999]
}

void TimeStamp::Write( std::ostream& Stream ) const {
	char Output[20] = "00:00:00 00-00-0000";

	WriteReversedValue( Second, 2, Output );
	WriteReversedValue( Minute, 2, Output + 3 );
	WriteReversedValue( Hour, 2, Output + 6 );
	WriteReversedValue( Day, 2, Output + 9 );
	WriteReversedValue( Month, 2, Output + 12 );
	WriteReversedValue( Year, 4, Output + 15 );

	std::reverse( Output, Output + sizeof(Output)-1 );
	Stream.write( Output, sizeof(Output)-1 );
}

void TimeStamp::WritePrecise( std::ostream& Stream ) const {
	char Output[24] = "000.00:00:00 00-00-0000";

	WriteReversedValue( Millisecond, 3, Output );
	WriteReversedValue( Second, 2, Output + 4 );
	WriteReversedValue( Minute, 2, Output + 7 );
	WriteReversedValue( Hour, 2, Output + 10 );
	WriteReversedValue( Day, 2, Output + 13 );
	WriteReversedValue( Month, 2, Output + 16 );
	WriteReversedValue( Year, 4, Output + 19 );

	std::reverse( Output, Output + sizeof(Output)-1 );
	Stream.write( Output, sizeof(Output)-1 );
}

std::ostream& operator<<(std::ostream& Stream, TimeStamp const& TS) {
	TS.WritePrecise(Stream);
	return Stream;
}
