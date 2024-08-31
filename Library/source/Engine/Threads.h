#pragma once
#include "Engine/StandardTypes.h"

/** Create a thread that will execute the callable object */
template<typename Callable>
std::jthread CreateThread(Callable& callable) {
	if constexpr (std::invocable<Callable, std::stop_token>) return std::jthread{ std::bind_front(&Callable::operator(), &callable) };
	else return std::jthread{ &Callable::operator(), &callable };
}

/** Helper type that allows access to a variable that has be locked by a mutex in an inclusive way, so other threads can also access it at the same time. */
template<typename ValueType_, typename MutexType = std::shared_mutex>
struct InclusiveLockedValue {
	using ValueType = ValueType_;
	using LockType = std::shared_lock<MutexType>;

	InclusiveLockedValue(ValueType const& value, MutexType& mutex) : value(value), lock(mutex) {}
	InclusiveLockedValue(ValueType const& value, LockType&& lock) : value(value), lock(std::move(lock)) {}
	InclusiveLockedValue(InclusiveLockedValue const&) = delete;
	InclusiveLockedValue(InclusiveLockedValue&&) = default;

	inline ValueType const* operator->() const { return &value; }
	inline ValueType const& operator*() const { return value; }

	inline auto const& operator[](size_t index) const {
		if constexpr (stdext::indexible<ValueType>) return value[index];
		else return value;
	}

private:
	ValueType const& value;
	LockType lock;
};

/** Helper type that allows access to a variable that has be locked by a mutex in an exclusive way, so other threads cannot access it */
template<typename ValueType_, typename MutexType = std::shared_mutex>
struct ExclusiveLockedValue {
	using ValueType = ValueType_;
	using LockType = std::unique_lock<MutexType>;

	ExclusiveLockedValue(ValueType& value, MutexType& mutex) : value(value), lock(mutex) {}
	ExclusiveLockedValue(ValueType& value, LockType&& lock) : value(value), lock(std::move(lock)) {}
	ExclusiveLockedValue(ExclusiveLockedValue const&) = delete;
	ExclusiveLockedValue(ExclusiveLockedValue&&) = default;

	inline ValueType* operator->() { return &value; }
	inline ValueType& operator*() { return value; }
	inline ValueType const* operator->() const { return &value; }
	inline ValueType const& operator*() const { return value; }

	inline auto& operator[](size_t index) {
		if constexpr (stdext::indexible<ValueType>) return value[index];
		else return value;
	}
	inline auto const& operator[](size_t index) const {
		if constexpr (stdext::indexible<ValueType>) return value[index];
		else return value;
	}

private:
	ValueType& value;
	LockType lock;
};

/** Wraps a type to require mutex-based locking before it can be accessed, ensuring thread-safety. */
template<typename ValueType, typename MutexType = std::shared_mutex>
struct ThreadSafe {
	using Inclusive = InclusiveLockedValue<ValueType, MutexType>;
	using Exclusive = ExclusiveLockedValue<ValueType, MutexType>;

	ThreadSafe() = default;
	ThreadSafe(const ValueType& value) : value(value) {}
	ThreadSafe(ValueType&& value) : value(std::move(value)) {}

	template<typename... ArgTypes>
	ThreadSafe(ArgTypes&&... arguments) : value(std::forward<ArgTypes>(arguments)...) {}

	/** Get an inclusive lock, allowing read-only access to the value which may be shared by other threads. */
	[[nodiscard]] inline Inclusive LockInclusive() const { return Inclusive{ value, mutex }; }
	/** Get an exclusive lock, allowing read-write access to the value which for only the calling thread. */
	[[nodiscard]] inline Exclusive LockExclusive() { return Exclusive{ value, mutex }; }

protected:
	ValueType value;
	mutable MutexType mutex;
};

/** Similar to ThreadSafe, but systems can wait until notified that the value has changed. */
template<typename ValueType, typename MutexType = std::shared_mutex>
struct TriggeredThreadSafe : public ThreadSafe<ValueType, MutexType> {
	using ThreadSafe<ValueType, MutexType>::ThreadSafe;
	using Inclusive = ThreadSafe<ValueType, MutexType>::Inclusive;
	using Exclusive = ThreadSafe<ValueType, MutexType>::Exclusive;

	~TriggeredThreadSafe() {
		//Always notify during destruction so that waiting threads can update and see that they are finished.
		Notify();
	}

	/** Notify waiting threads that the value has been modified. Can be called while a lock is held. */
	inline void Notify() { cv.notify_all(); }
	/** Nofity a single waiting thread that the value has been modified. Can be called while a lock is held. */
	inline void NotifySingle() { cv.notify_one(); }

	/** Causes the calling thread to wait until the predicate returns true. */
	template<std::predicate<ValueType const&> PredicateType>
	[[nodiscard]] inline Inclusive WaitInclusive(PredicateType&& predicate) const {
		typename Inclusive::LockType lock{ this->mutex };
		cv.wait(lock, [&]() { return predicate(this->value); });
		return Inclusive{ this->value, std::move(lock) };
	}
	template<std::predicate<ValueType const&> PredicateType>
	[[nodiscard]] inline Exclusive WaitExclusive(PredicateType&& predicate) {
		typename Exclusive::LockType lock{ this->mutex };
		cv.wait(lock, [&]() { return predicate(this->value); });
		return Exclusive{ this->value, std::move(lock) };
	}

	/** Causes the calling thread to wait until the predicate returns true, or until the duration has elapsed. If the duration elapses and the predicate is still false, does not return a lock. */
	template<std::predicate<ValueType const&> PredicateType>
	[[nodiscard]] inline std::optional<Inclusive> WaitInclusive(std::chrono::milliseconds duration, PredicateType&& predicate) const {
		typename Inclusive::LockType lock{ this->mutex };
		if (cv.wait_for(lock, duration, [&]() { return predicate(this->value); })) return Inclusive{ this->value, std::move(lock) };
		else return std::nullopt;
	}
	template<std::predicate<ValueType const&> PredicateType>
	[[nodiscard]] inline std::optional<Exclusive> WaitExclusive(std::chrono::milliseconds duration, PredicateType&& predicate) {
		typename Exclusive::LockType lock{ this->mutex };
		if (cv.wait_for(lock, duration, [&]() { return predicate(this->value); })) return Exclusive{ this->value, std::move(lock) };
		else return std::nullopt;
	}

	/** Causes the calling thread to wait until the predicate returns true, or until the time is reached. If the time is reached and the predicate is still false, does not return a lock. */
	template<std::predicate<ValueType const&> PredicateType>
	[[nodiscard]] inline std::optional<Inclusive> WaitInclusive(std::chrono::high_resolution_clock::time_point time, PredicateType&& predicate) const {
		typename Inclusive::LockType lock{ this->mutex };
		if (cv.wait_until(lock, time, [&]() { return predicate(this->value); })) return Inclusive{ this->value, std::move(lock) };
		else return std::nullopt;
	}
	template<std::predicate<ValueType const&> PredicateType>
	[[nodiscard]] inline std::optional<Exclusive> WaitExclusive(std::chrono::high_resolution_clock::time_point time, PredicateType&& predicate) {
		typename Exclusive::LockType lock{ this->mutex };
		if (cv.wait_until(lock, time, [&]() { return predicate(this->value); })) return Exclusive{ this->value, std::move(lock) };
		else return std::nullopt;
	}

protected:
	mutable std::condition_variable_any cv;
};
