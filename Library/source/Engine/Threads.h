#pragma once
#include "Engine/StandardTypes.h"

/**
 * Wraps a type to require mutex-based locking before it can be accessed, ensuring thread-safety.
 * Access is neither copyable nor movable. It is specific to the scope in which it is created.
 */
template<typename T>
struct ThreadSafe {
	struct Inclusive {
		inline T const* operator->() const { return &value; }
		inline T const& operator*() const { return value; }

	private:
		friend ThreadSafe;
		Inclusive(T const& value, std::shared_mutex& mutex) : value(value), lock(mutex) {}
		Inclusive(Inclusive const&) = delete;
		Inclusive(Inclusive&&) = delete;

		T const& value;
		std::shared_lock<std::shared_mutex> lock;
	};

	struct Exclusive {
		inline T* operator->() { return &value; }
		inline T& operator*() { return value; }
		inline T const* operator->() const { return &value; }
		inline T const& operator*() const { return value; }

	private:
		friend ThreadSafe;
		Exclusive(T& value, std::shared_mutex& mutex) : value(value), lock(mutex) {}
		Exclusive(Exclusive const&) = delete;
		Exclusive(Exclusive&&) = delete;

		T& value;
		std::unique_lock<std::shared_mutex> lock;
	};

	/** Get an inclusive lock, allowing read-only access to the value which may be shared by other threads. */
	[[nodiscard]] inline Inclusive LockInclusive() const { return Inclusive{ value, mutex }; }
	/** Get an exclusive lock, allowing read-write access to the value which for only the calling thread. */
	[[nodiscard]] inline Exclusive LockExclusive() { return Exclusive{ value, mutex }; }

private:
	T value;
	mutable std::shared_mutex mutex;
};
