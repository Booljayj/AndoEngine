#include "Engine/Utility.h"

namespace Utility {
	uint8_t WriteReversedValue(uint64_t value, char* buffer, size_t size) {
		if (size > 0) {
			char const* const end = buffer + size;
			size_t digits = 0;

			do {
				*(buffer + digits) = value % 10 + '0'; //Write the lowest digit
				value /= 10; //Shift all digits down
				++digits;
			} while (value > 0 && buffer != end);

			return digits;
		}
		return 0;
	}

	uint8_t WriteReversedValue(int64_t value, char* buffer, size_t size) {
		uint8_t const digits = WriteReversedValue(static_cast<uint64_t>(value), buffer, size);
		if (digits < size && value < 0) {
			*(buffer + digits) = '-'; //Write the negative sign
			return digits + 1;
		} else {
			return digits;
		}
	}
}

std::ostream& operator<<(std::ostream& stream, ByteSize bytes) {
	const auto WriteSmallUnit = [&stream](uint64_t size) {
		char output[6] = "B    ";
		const size_t digits = Utility::WriteReversedValue(size, output + 2, 3);
		const size_t outputSize = std::min<uint8_t>(sizeof(output)-1, digits + 2);

		std::reverse(output, output + outputSize);
		stream.write(output, outputSize);
	};
	const auto WriteUnit = [&stream](char Suffix, uint64_t fixedPointSize) {
		char output[9] = "Bx 0.x  ";
		output[1] = Suffix;
		Utility::WriteReversedValue(fixedPointSize % 10, output + 3, 1);
		const size_t digits = Utility::WriteReversedValue(fixedPointSize / 10, output + 5, 3);
		const size_t outputSize = std::min<uint8_t>(sizeof(output)-1, digits + 5);

		std::reverse(output, output + outputSize);
		stream.write(output, outputSize);
	};

	size_t size = bytes.size;
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
	return stream;
}
