#pragma once
#include <array>
#include <cstdint>
#include <chrono>
#include <ostream>

/** A broken-down time point used for displaying calendar information. Value ranges are set up for common usage, not for extremes. TimeStamps are not meant to be manually edited. */
union TimeStamp {
private:
	static constexpr uint8_t YEAR_BITS = 14;
	static_assert((1ULL<<YEAR_BITS)-1 >= 9999, "Number of bits cannot contain required value range");

	static constexpr uint8_t MONTH_BITS = 4;
	static_assert((1ULL<<MONTH_BITS)-1 >= 12, "Number of bits cannot contain required value range");

	static constexpr uint8_t DAY_BITS = 5;
	static_assert((1ULL<<DAY_BITS)-1 >= 31, "Number of bits cannot contain required value range");

	static constexpr uint8_t HOUR_BITS = 5;
	static_assert((1ULL<<HOUR_BITS)-1 >= 23, "Number of bits cannot contain required value range");

	static constexpr uint8_t MINUTE_BITS = 6;
	static_assert((1ULL<<MINUTE_BITS)-1 >= 59, "Number of bits cannot contain required value range");

	static constexpr uint8_t SECOND_BITS = 6;
	static_assert((1ULL<<SECOND_BITS)-1 >= 60, "Number of bits cannot contain required value range");

	static constexpr uint8_t MILLISECOND_BITS = 10;
	static_assert((1ULL<<MILLISECOND_BITS)-1 >= 999, "Number of bits cannot contain required value range");

	/** Write a value to a buffer using an exact number of digits in reverse order */
	static void WriteReversedValue( uint16_t Value, const uint8_t NumDigits, char* Buffer) noexcept;

public:
	using StorageType = uint64_t;

	template <uint8_t OFFSET, uint8_t BITS>
	struct TMember {
		static constexpr uint8_t Offset = OFFSET;
		static constexpr uint8_t Bits = BITS;
		static constexpr StorageType Maximum = (StorageType(1) << Bits)-1;
		static constexpr StorageType Mask = Maximum << Offset;

		static_assert(Bits > 0, "Bits cannot be 0");
		static_assert(Bits < (sizeof(StorageType)*8), "Can't fill entire bitfield with one member");
		static_assert((Offset+Bits) <= (sizeof(StorageType)*8), "Member exceeds bitfield boundaries");

		operator StorageType() const {
			return (_Value >> Offset) & Maximum;
		}

		TMember& operator=(StorageType V) {
			_Value = (_Value & ~Mask) | ((V & Maximum) << Offset);
			return *this;
		}

	private:
		StorageType _Value;
	};

	StorageType Value = 0;

	// Bit layout for a timestamp is as follows:
	//  7        6        5        4        3        2        1        0
	//  xxxxxxxx-xxxxxxpp-pppppppp-ssssssmm-mmmmhhhh-hDDDDDMM-MMYYYYYY-YYYYYYYY
	//  |64            |50         |40   |34    |28   |23  |18  |14            |0
	static constexpr uint64_t DefaultValue =
	  0b00000000'00000000'00000000'00000000'00000000'00000100'01000000'00000000;

	/** [0-9999] the gregorian calendar year */
	TMember<0, YEAR_BITS> Year;
	/** [1-12] the month number */
	TMember<YEAR_BITS, MONTH_BITS> Month;
	/** [1-31] the day in the current month */
	TMember<YEAR_BITS+MONTH_BITS, DAY_BITS> Day;

	/** [0-23] the hour of the day */
	TMember<YEAR_BITS+MONTH_BITS+DAY_BITS, HOUR_BITS> Hour;
	/** [0-59] the minute of the hour */
	TMember<YEAR_BITS+MONTH_BITS+DAY_BITS+HOUR_BITS, MINUTE_BITS> Minute;
	/** [0-60] the second of the minute (60 is used for leap seconds in some cases) */
	TMember<YEAR_BITS+MONTH_BITS+DAY_BITS+HOUR_BITS+MINUTE_BITS, SECOND_BITS> Second;
	/** [0-999] the milliseconds past the second */
	TMember<YEAR_BITS+MONTH_BITS+DAY_BITS+HOUR_BITS+MINUTE_BITS+SECOND_BITS, MILLISECOND_BITS> Millisecond;

	static TimeStamp Now() noexcept;

	TimeStamp() noexcept;
	TimeStamp(uint64_t InValue) noexcept;
	TimeStamp(std::chrono::system_clock::time_point const& TimePoint) noexcept;

	/** Write this timestamp to the stream in the form "YYYY-MM-DD HH:MM:SS" */
	void Write( std::ostream& Stream ) const;
	/** Write this timestamp to the stream in the form "YYYY-MM-DD HH:MM:SS.sss" */
	void WritePrecise( std::ostream& Stream ) const;
};

static_assert(sizeof(TimeStamp)==sizeof(TimeStamp::StorageType), "Invalid TimeStamp union packing, TimeStamp must be exactly equal in size to the internal storage type");

std::ostream& operator<<(std::ostream&, TimeStamp const&);
