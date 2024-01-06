#include "Engine/Logging/Logger.h"
#include "Engine/Algo.h"
#include "Engine/Logging/LogDevice.h"
#include "Engine/Logging/LogMessage.h"

/** Allows log devices to process log messages on a separate thread */
struct LogWorker {
	/** Variables shared with */
	struct Shared {
		std::unique_ptr<LogMessageQueue>& front;
		std::mutex& mutex;
		std::condition_variable& cv;
	};

	LogWorker(Shared const& shared, std::vector<std::shared_ptr<ILogDevice>> const& devices)
		: shared(shared), devices(devices), back(std::make_unique<LogMessageQueue>())
	{}

	void operator()(std::stop_token token) {
		while (!token.stop_requested()) {
			WaitSwapQueues(token);
			devices << *back;
		}

		//A stop was requested at some point before or during the previous iteration.
		//We may still have some final messages in the queue, so process those before fully exiting.
		LockSwapQueues();
		devices << *back;
	}

private:
	Shared shared;
	std::vector<std::shared_ptr<ILogDevice>> devices;
	std::unique_ptr<LogMessageQueue> back;

	void WaitSwapQueues(std::stop_token const& token) {
		//Wait until something is added to the queue.
		//Proceed immediately if the queue is currently not empty or if we want to shut down this worker
		std::unique_lock lock{ shared.mutex };
		shared.cv.wait(lock, [this, &token]() { return shared.front->Size() > 0 || token.stop_requested(); });
		//Swap to capture the queue as quickly as possible and release the lock
		std::swap(shared.front, back);
	}

	void LockSwapQueues() {
		std::lock_guard lock{ shared.mutex };
		std::swap(shared.front, back);
	}
};

Logger Logger::instance;
thread_local std::string Logger::scratch;

Logger::Logger()
	: queue(std::make_unique<LogMessageQueue>())
{}

Logger::~Logger() {
	std::lock_guard const lock{ mutex.thread };

	StopWorkerThread();
}

void Logger::AddDevices(std::span<std::shared_ptr<ILogDevice> const> view) {
	std::lock_guard const lock{ mutex.thread };

	stdext::append(devices, view);
	RestartWorkerThread();
}

void Logger::RemoveDevices(std::span<std::shared_ptr<ILogDevice> const> view) {
	std::lock_guard const lock{ mutex.thread };

	Algo::RemoveSwap(devices, view);
	RestartWorkerThread();
}

void Logger::PushFormatted(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::string_view format, std::format_args const& args) noexcept {
	if (verbosity <= category.GetMaxVerbosity()) {
		//Perform formatting on the calling thread
		scratch.clear();
		std::vformat_to(std::back_inserter(scratch), format, args);

		//Push the output onto the queue. We'll lock the mutex as briefly as possible, copying the header and swapping the message memory.
		{
			std::lock_guard const lock{ mutex.queue };
			queue->PushSwap(
				LogMessageHeader{ ClockTimeStamp::Now(), verbosity, &category, location },
				scratch
			);
		}

		//Notify the writer thread that there are new messages in the queue
		cv.notify_one();
	}
}

void Logger::PushUnformatted(LogCategory const& category, ELogVerbosity verbosity, std::source_location location, std::string_view message) noexcept {
	if (verbosity <= category.GetMaxVerbosity()) {
		//Copy the message on the calling thread
		scratch = message;

		//Push the output onto the queue. We'll lock the mutex as briefly as possible, copying the header and swapping the message memory.
		{
			std::lock_guard const lock{ mutex.queue };
			queue->PushSwap(
				LogMessageHeader{ ClockTimeStamp::Now(), verbosity, &category, location },
				scratch
			);
		}

		//Notify the writer thread that there are new messages in the queue
		cv.notify_one();
	}
}

void Logger::StopWorkerThread() {
	if (thread) {
		thread->request_stop();
		cv.notify_all();
	}
}

void Logger::RestartWorkerThread() {
	StopWorkerThread();

	LogWorker::Shared const shared{ queue, mutex.queue, cv };
	thread.emplace(LogWorker{ shared, devices });
}
