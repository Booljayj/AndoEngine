#include <cstdint>
#include <string>
#include <fstream>
#include <memory>
#include <shared_mutex>
#include <thread>
#include "Engine/Context.h"
#include "Engine/Logging/LogCategory.h"
#include "Profiling/ProfileTypes.h"

DECLARE_LOG_CATEGORY(Profiler);

namespace Profiling {
	/** Contains utilities for profiling various processes within a session and outputting the results to a file */
	struct Profiler {
	public:
		/** Get the singleton profiler instance */
		static Profiler& Get();

		/**
		 * Begin a new profiling session with the provided name. Results will be output to a file including the session name.
		 * Multiple sessions are not allowed, starting a new session will end the previous one.
		 */
		bool BeginSession(CTX_ARG, std::string_view name);
		/** End the current profiling session, if any. */
		void EndSession();

		/** Returns true if there is currently an active session */
		bool IsSessionRunning();
		/** Gets the name of the current session */
		std::string GetSessionName();
		/** Gets the start time of the current session */
		TimePointType GetSessionBeginTimePoint();

		/** Write profiling event information to the current session */
		void WriteInstantEvent(std::string_view name, TimePointType time);
		void WriteDurationEvent(std::string_view name, TimePointType time, DurationType duration);
		void WriteCounterEvent(std::string_view name, TimePointType time, uint64_t value);
		void WriteObjectCreationEvent(std::string_view name, void const* address, TimePointType time);
		void WriteObjectDestructionEvent(std::string_view name, void const* address, TimePointType time);
		void WriteObjectSnapshotEvent(std::string_view name, void const* address, std::string_view snapshot, TimePointType time);

	private:
		struct Session {
			std::string name;
			TimePointType beginTimePoint;

			std::ofstream file;
			uint16_t flushCounter;

			static uint32_t GetThreadID();

			Session(CTX_ARG, std::string_view inName);
			~Session();

			bool IsValid() const;
			void IncrementFlushCounter();

			void WriteInstantEvent(std::string_view name, TimePointType time);
			void WriteDurationEvent(std::string_view name, TimePointType time, DurationType duration);
			void WriteCounterEvent(std::string_view name, TimePointType time, uint64_t value);
			void WriteObjectCreationEvent(std::string_view name, void const* address, TimePointType time);
			void WriteObjectDestructionEvent(std::string_view name, void const* address, TimePointType time);
			void WriteObjectSnapshotEvent(std::string_view name, void const* address, std::string_view snapshot, TimePointType time);
		};

		std::shared_mutex sessionMutex;
		std::unique_ptr<Session> session;

		Profiler() = default;
	};
}