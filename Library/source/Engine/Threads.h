#pragma once
#include "Engine/StandardTypes.h"

/**
 * Wraps a type to require mutex-based locking before it can be accessed, ensuring thread-safety.
 * Access is neither copyable nor movable. It is specific to the scope in which it is created.
 */
template<typename ValueType, typename MutexType = std::shared_mutex>
struct ThreadSafe {
	struct Inclusive {
		inline ValueType const* operator->() const { return &value; }
		inline ValueType const& operator*() const { return value; }

	private:
		friend ThreadSafe;
		Inclusive(ValueType const& value, MutexType& mutex) : value(value), lock(mutex) {}
		Inclusive(Inclusive const&) = delete;
		Inclusive(Inclusive&&) = delete;

		ValueType const& value;
		std::shared_lock<MutexType> lock;
	};

	struct Exclusive {
		inline ValueType* operator->() { return &value; }
		inline ValueType& operator*() { return value; }
		inline ValueType const* operator->() const { return &value; }
		inline ValueType const& operator*() const { return value; }

	private:
		friend ThreadSafe;
		Exclusive(ValueType& value, MutexType& mutex) : value(value), lock(mutex) {}
		Exclusive(Exclusive const&) = delete;
		Exclusive(Exclusive&&) = delete;

		ValueType& value;
		std::unique_lock<MutexType> lock;
	};

	ThreadSafe() = default;
	ThreadSafe(const ValueType& value) : value(value) {}
	ThreadSafe(ValueType&& value) : value(std::move(value)) {}

	template<typename... ArgTypes>
	ThreadSafe(ArgTypes&&... arguments) : value(std::forward<ArgTypes>(arguments)...) {}

	/** Get an inclusive lock, allowing read-only access to the value which may be shared by other threads. */
	[[nodiscard]] inline Inclusive LockInclusive() const { return Inclusive{ value, mutex }; }
	/** Get an exclusive lock, allowing read-write access to the value which for only the calling thread. */
	[[nodiscard]] inline Exclusive LockExclusive() { return Exclusive{ value, mutex }; }

private:
	ValueType value;
	mutable MutexType mutex;
};
