#pragma once
#include "Engine/Array.h"
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogVerbosity.h"
#include "Engine/Core.h"
#include "Engine/String.h"
#include "Engine/TimeStamp.h"

struct LogMessageHeader {
	ClockTimeStamp time;
	ELogVerbosity verbosity;
	LogCategory const* category;
	std::source_location location;
};
static_assert(std::is_trivially_copyable_v<LogMessageHeader>, "LogMessageHeader should be trivially copyable");
static_assert(std::is_trivially_destructible_v<LogMessageHeader>, "LogMessageHeader should be trivially destructible");

struct LogMessage {
	LogMessageHeader header;
	std::string message;
};

/** A dynamic queue of log output, which reuses allocated memory as much as possible */
struct LogMessageQueue {
	static constexpr size_t ElementBlockSize = 512;

	LogMessageQueue() : elements(ElementBlockSize) {}
	LogMessageQueue(const LogMessageQueue&) = delete;
	LogMessageQueue(LogMessageQueue&&) = delete;

	inline auto begin() const { return elements.begin(); }
	inline auto end() const { return elements.begin() + size; }

	inline size_t Size() const { return size; }
	
	void PushSwap(LogMessageHeader const& header, std::string& message);
	inline void Reset() { size = 0; }

private:
	std::vector<LogMessage> elements;
	size_t size = 0;
};
