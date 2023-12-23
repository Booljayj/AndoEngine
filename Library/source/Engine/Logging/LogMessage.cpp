#include "Engine/Logging/LogMessage.h"

void LogMessageQueue::PushSwap(LogMessageHeader const& header, std::string& message) {
	//If we don't have enough space for this new message, add another block of entries to the queue
	//We work with blocks to reduce the frequency that this will happen
	if (elements.size() <= size) elements.resize(elements.size() + ElementBlockSize);

	elements[size].header = header;
	std::swap(elements[size].message, message);
	++size;
}
