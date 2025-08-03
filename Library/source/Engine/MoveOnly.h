#pragma once

/**
 * A value that can only be moved from the owning struct, not copied. When moved, the old variable will be set to the default-initialized value.
 * The underlying value is not required to be move-only, and can be copied when accessed directly.
 * The primary use of this is to ensure that a struct uses move semantics for ownership-related variables when using the default move constructor.
 */
template<std::semiregular T>
struct MoveOnly {
	explicit MoveOnly(T const& value) : value(value) {}
	explicit MoveOnly(T&& value) : value(value) {}

	MoveOnly(MoveOnly const&) = delete;
	MoveOnly(MoveOnly&& other) : value(other.value) { other.value = T{}; }

	MoveOnly& operator=(MoveOnly&& other) {
		value = other.value;
		other.value = T{};
		return *this;
	}

	operator T() const { return value; }

	T& get() { return value; }
	const T& get() const { return value; }

private:
	T value = T{};
};
