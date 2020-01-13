#include "Utility.h"

namespace Utility {
	void WriteReversedValue(uint16_t value, const uint8_t numDigits, char* buffer) {
		//Generate the digits in reverse order
		char const* const end = buffer+numDigits;
		for (; value > 0 && buffer < end; ++buffer) {
			*buffer = (value % 10) + '0'; //Write the lowest digit
			value /= 10; //Shift all digits down
		}
	}

	uint8_t WriteMinimalReversedValue(uint16_t value, const uint8_t maxNumDigits, char* buffer) {
		//Generate the digits in reverse order
		char const* const end = buffer+maxNumDigits;
		uint8_t numDigitsWritten = 0;
		for (; value > 0 && buffer < end; ++buffer) {
			numDigitsWritten += uint8_t(value > 0);
			*buffer = (value % 10) + '0'; //Write the lowest digit
			value /= 10; //Shift all digits down
		}
		return std::max<uint8_t>(numDigitsWritten, 1);
	}

	void WriteByteSizeValue(std::ostream& stream, uint64_t size) {
		const auto WriteSmallUnit = [&stream](uint64_t size) {
			char output[6] = "B    ";
			const uint8_t digits = WriteMinimalReversedValue(size, 3, output + 2);
			const uint8_t outputSize = std::min<uint8_t>(sizeof(output)-1, digits + 2);

			std::reverse(output, output + outputSize);
			stream.write(output, outputSize);
		};
		const auto WriteUnit = [&stream](char Suffix, uint64_t fixedPointSize) {
			char output[9] = "Bx 0.x  ";
			output[1] = Suffix;
			WriteReversedValue(fixedPointSize % 10, 1, output + 3);
			const uint8_t digits = WriteMinimalReversedValue(fixedPointSize / 10, 3, output + 5);
			const uint8_t outputSize = std::min<uint8_t>(sizeof(output)-1, digits + 5);

			std::reverse(output, output + outputSize);
			stream.write(output, outputSize);
		};

		if (size < 1000L) {
			WriteSmallUnit(size);
		} else if (size < 999'950L) {
			WriteUnit('k', size / 100);
		} else if (size < 999'950'000L) {
			WriteUnit('M', size / 100'000);
		} else if (size < 999'950'000'000L) {
			WriteUnit('G', size / 100'000'000);
		} else if (size < 999'950'000'000'000L) {
			WriteUnit('T', size / 100'000'000'000);
		} else if (size < 999'950'000'000'000'000L) {
			WriteUnit('P', size / 100'000'000'000'000);
		} else {
			WriteUnit('E', size / 100'000'000'000'000'000);
		}
	}
}
