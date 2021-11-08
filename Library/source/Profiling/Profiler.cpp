#include "Profiling/Profiler.h"
#include "Engine/LogCommands.h"

DEFINE_LOG_CATEGORY(Profiler, Info);

namespace Profiling {
	Profiler& Profiler::Get() {
		static Profiler instance;
		return instance;
	}

	bool Profiler::BeginSession(CTX_ARG, std::string_view name) {
		if (name.empty()) {
			name = "Profile"sv;
		}

		const std::unique_lock lock{ sessionMutex };

		session = std::make_unique<Session>(CTX, name);

		if (!session || !session->IsValid()) {
			LOGF(Profiler, Error, "Could not start profiling session '%s'.", name);
			session.reset();
			return false;
		}
		return true;
	}

	void Profiler::EndSession() {
		const std::unique_lock lock{ sessionMutex };
		session.reset();
	}

	bool Profiler::IsSessionRunning() {
		const std::shared_lock lock{ sessionMutex };
		return !!session;
	}

	std::string Profiler::GetSessionName() {
		const std::shared_lock lock{ sessionMutex };
		if (session) return session->name;
		else return "None";
	}

	TimePointType Profiler::GetSessionBeginTimePoint() {
		const std::shared_lock lock{ sessionMutex };
		if (session) return session->beginTimePoint;
		else return TimePointType::min();
	}

	void Profiler::WriteInstantEvent(std::string_view name, const ProfileCategory& category, TimePointType time) {
		const std::unique_lock lock{ sessionMutex };
		if (session) session->WriteInstantEvent(name, category, time);
	}

	void Profiler::WriteDurationEvent(std::string_view name, const ProfileCategory& category, TimePointType startTime, DurationType duration) {
		const std::unique_lock lock{ sessionMutex };
		if (session) session->WriteDurationEvent(name, category, startTime, duration);
	}

	void Profiler::WriteCounterEvent(std::string_view name, const ProfileCategory& category, TimePointType time, uint64_t value) {
		const std::unique_lock lock{ sessionMutex };
		if (session) session->WriteCounterEvent(name, category, time, value);
	}

	void Profiler::WriteObjectCreationEvent(std::string_view name, const ProfileCategory& category, void const* address, TimePointType time) {
		const std::unique_lock lock{ sessionMutex };
		if (session) session->WriteObjectCreationEvent(name, category, address, time);
	}

	void Profiler::WriteObjectDestructionEvent(std::string_view name, const ProfileCategory& category, void const* address, TimePointType time) {
		const std::unique_lock lock{ sessionMutex };
		if (session) session->WriteObjectDestructionEvent(name, category, address, time);
	}

	void Profiler::WriteObjectSnapshotEvent(std::string_view name, const ProfileCategory& category, void const* address, std::string_view snapshot, TimePointType time) {
		const std::unique_lock lock{ sessionMutex };
		if (session) session->WriteObjectSnapshotEvent(name, category, address, snapshot, time);
	}

	uint32_t Profiler::Session::GetThreadID() {
		return std::hash<std::thread::id>{}(std::this_thread::get_id());
	}

	Profiler::Session::Session(CTX_ARG, std::string_view inName)
	: name(inName)
	{
		beginTimePoint = Profiling::Now();
		flushCounter = 5;

		static uint32_t counter = 0;
		const std::string_view filename = t_printf("%s_%u_%u.profile.json", name.c_str(), beginTimePoint.time_since_epoch().count(), counter++);

		file.open(filename.data(), std::ios_base::out | std::ios_base::trunc);

		if (file.is_open()) {
			//Write the file header
			const uint32_t threadID = GetThreadID();
			file
				<< "[{\"ph\":\"I\",\"cat\":\"Default\",\"pid\":0,\"name\":\""sv << name
				<< " start\",\"tid\":"sv << threadID
				<< ",\"ts\":0}\n"sv;

			IncrementFlushCounter();
			LOGF(Profiler, Info, "Started profiling session for file: %s", filename.data());

		} else {
			LOGF(Profiler, Error, "Could not open profiler output file: %s. Session not started.", filename.data());
		}
	}

	Profiler::Session::~Session() {
		if (file.is_open()) {
			file << ']';
			file.flush();
			file.close();
		}
	}

	bool Profiler::Session::IsValid() const {
		return file.is_open();
	}

	void Profiler::Session::IncrementFlushCounter() {
		++flushCounter;
		if (flushCounter > 0) {
			file.flush();
			flushCounter = 5;
		}
	}

	void Profiler::Session::WriteInstantEvent(std::string_view name, const ProfileCategory& category, TimePointType time) {
		const uint32_t threadID = GetThreadID();
		const uint64_t timeMicroseconds = (time - beginTimePoint).count();
		file
			<< ",{\"ph\":\"I\",\"pid\":0,\"name\":\""sv << name
			<< "\",\"cat\":\""sv << category.GetName()
			<< "\",\"tid\":"sv << threadID
			<< ",\"ts\":"sv << timeMicroseconds
			<< "}\n"sv;

		IncrementFlushCounter();
	}

	void Profiler::Session::WriteDurationEvent(std::string_view name, const ProfileCategory& category, TimePointType time, DurationType duration) {
		const uint32_t threadID = GetThreadID();
		const uint64_t timeMicroseconds = (time - beginTimePoint).count();
		const uint64_t durationMicroseconds = duration.count();
		file
			<< ",{\"ph\":\"X\",\"pid\":0,\"name\":\""sv << name
			<< "\",\"cat\":\""sv << category.GetName()
			<< "\",\"tid\":"sv << threadID
			<< ",\"ts\":"sv << timeMicroseconds
			<< ",\"dur\":"sv << durationMicroseconds
			<< "}\n"sv;

		IncrementFlushCounter();
	}

	void Profiler::Session::WriteCounterEvent(std::string_view name, const ProfileCategory& category, TimePointType time, uint64_t value) {
		const uint32_t threadID = GetThreadID();
		const uint64_t timeMicroseconds = (time - beginTimePoint).count();
		file
			<< ",{\"ph\":\"C\",\"pid\":0,\"name\":\""sv << name
			<< "\",\"cat\":\""sv << category.GetName()
			<< "\",\"tid\":"sv << threadID
			<< ",\"ts\":"sv << timeMicroseconds
			<< ",\"args\":{\""sv << name << "\":" << value
			<< "}}\n"sv;

		IncrementFlushCounter();
	}

	void Profiler::Session::WriteObjectCreationEvent(std::string_view name, const ProfileCategory& category, void const* address, TimePointType time) {
		//@todo
	}
	void Profiler::Session::WriteObjectDestructionEvent(std::string_view name, const ProfileCategory& category, void const* address, TimePointType time) {
		//@todo
	}
	void Profiler::Session::WriteObjectSnapshotEvent(std::string_view name, const ProfileCategory& category, void const* address, std::string_view snapshot, TimePointType time) {
		//@todo
	}
}
