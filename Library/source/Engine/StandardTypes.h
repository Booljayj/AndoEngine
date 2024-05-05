#pragma once
/*
 * This file contains standard types that should be precompiled.
 *
 * This includes many commonly-used elements from the Standard Template Library, fundamental external libraries like math utilities,
 * and custom types that are broadly used and don't have many other dependencies.
 */

//============================================================
// STL types

//Basics
#include <cassert>
#include <type_traits>

//Primitive types
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
using namespace std::string_view_literals;

//Container types
//Array
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <queue>
#include <stack>
#include <vector>
//Map
#include <map>
#include <unordered_map>
//Set
#include <set>
#include <unordered_set>

//Container operations
#include <algorithm>
#include <iterator>
#include <ranges>
#include <span>

//Utility types
#include <expected>
#include <tuple>
#include <optional>
#include <variant>

//Smart Pointers
#include <memory>

//Streams
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

//Threading
#include <atomic>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <thread>

//Localization
#include <locale>
#include <codecvt>

//Files
#include <filesystem>
#include <source_location>

//Mathematical
#include <cmath>
#include <limits>
#include <numbers>
#include <numeric>

//Misc
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <concepts>
#include <initializer_list>
#include <typeinfo>
#include <utility>

//Third-party and upcoming
#include "ThirdParty/uuid.h"
#include <absl/functional/function_ref.h>
#include <boost/endian.hpp>

namespace ranges = std::ranges;

//Custom extensions to the standard library
namespace stdext {
	/** A type that is an enum */
	template<typename T>
	concept enumeration = std::is_enum_v<T>;

	/** An integral or numeric type */
	template<typename T>
	concept numeric = std::integral<T> || std::floating_point<T>;

	/** A type of character which can be used to create a string */
	template<typename T>
	concept character = std::is_same_v<T, char> || std::is_same_v<T, wchar_t> || std::is_same_v<T, char8_t> || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

	/** A type which can be compared to nullptr, but is not literally nullptr */
	template<typename T>
	concept null_comparable =
		std::equality_comparable_with<std::nullptr_t, T> and
		std::copyable<T> and
		!std::is_null_pointer_v<T>;

	template<typename T, typename IndexType = size_t>
	concept indexible = requires(T t, IndexType i) {
		t[i];
	};

	constexpr inline size_t hash_combine(size_t a, size_t b) { return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2)); }

	/** Convert a byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T>
	constexpr std::span<T const> from_bytes(std::span<std::byte const> bytes) {
		if (bytes.size() % sizeof(T) != 0) throw std::runtime_error{ "invalid conversion, source byte span cannot be converted into a whole number of objects of the target type" };
		return std::span<T const>{ reinterpret_cast<T const*>(bytes.data()), bytes.size() / sizeof(T) };
	}
	/** Convert a byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T, size_t NumBytes>
		requires (NumBytes % sizeof(T) == 0)
	constexpr std::span<T const, NumBytes / sizeof(T)> from_bytes(std::span<std::byte const, NumBytes> bytes) {
		return std::span<T const, NumBytes / sizeof(T)>{ reinterpret_cast<T const*>(bytes.data()), bytes.size() / sizeof(T) };
	}
	/** Convert a writable byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T>
	constexpr std::span<T> from_writable_bytes(std::span<std::byte> bytes) {
		if (bytes.size() % sizeof(T) != 0) throw std::runtime_error{ "invalid conversion, source byte span cannot be converted into a whole number of objects of the target type" };
		return std::span<T>{ reinterpret_cast<T*>(bytes.data()), bytes.size() / sizeof(T) };
	}
	/** Convert a writable byte span into a span of objects. The byte span must be divisible into a whole number of objects */
	template<typename T, size_t NumBytes>
		requires (NumBytes % sizeof(T) == 0)
	constexpr std::span<T, NumBytes / sizeof(T)> from_writable_bytes(std::span<std::byte, NumBytes> bytes) {
		return std::span<T, NumBytes / sizeof(T)>{ reinterpret_cast<T*>(bytes.data()), bytes.size() / sizeof(T) };
	}

	/** A template callable which takes some parameters and does nothing. Can be used as a default argument for methods that take a callable. */
	template<typename T>
	struct no_op {
		void operator()(T) {}
	};

	template<typename SourceType, std::constructible_from<SourceType const&> TargetType, typename TargetAllocatorType, size_t SpanExtent>
	constexpr void append(std::vector<TargetType, TargetAllocatorType>& target, std::span<SourceType const, SpanExtent> const& source) {
		target.reserve(target.size() + source.size());
		for (SourceType const& element : source) target.emplace_back(element);
	}

	template<typename SourceType, std::constructible_from<SourceType&&> TargetType, typename SourceAllocatorType, typename TargetAllocatorType>
	void append(std::vector<TargetType, TargetAllocatorType>& target, std::vector<SourceType, SourceAllocatorType>&& source) {
		target.reserve(target.size() + source.size());
		for (SourceType& element : source) target.emplace_back(std::move(element));
		source.clear();
	}

	class recursive_shared_mutex : public std::shared_mutex {
	public:
		inline void lock(void) {
			std::thread::id this_id = std::this_thread::get_id();
			if (owner == this_id) {
				// recursive locking
				count++;
			} else {
				// normal locking
				shared_mutex::lock();
				owner = this_id;
				count = 1;
			}
		}

		inline void unlock(void) {
			if (count > 1) {
				// recursive unlocking
				count--;
			} else {
				// normal unlocking
				owner = std::thread::id();
				count = 0;
				shared_mutex::unlock();
			}
		}

	private:
		std::atomic<std::thread::id> owner;
		uint32_t count;
	};

	template<typename T>
	using function_ref = absl::FunctionRef<T>;

	/** Determines whether a return value should be a copy or a reference, in the context of returning a const value of type T */
	template<typename T>
	using value_or_reference_return_t = std::conditional_t<
		(sizeof(T) < 2 * sizeof(void*)) && std::is_trivially_copy_constructible_v<T>,
		const T,
		const T&
	>;

	/** Based on the Microsoft GSL implementation of not_null, but modified to use C++20 features and exceptions */
	template<null_comparable T>
	struct not_null {
		template<std::convertible_to<T> U>
		constexpr not_null(U&& u) noexcept(std::is_nothrow_move_constructible_v<T>) : ptr_(std::forward<U>(u)) {
			if (ptr_ == nullptr) throw std::runtime_error{ "not_null object was assigned to nullptr" };
		}

		constexpr not_null(T u) noexcept(std::is_nothrow_move_constructible_v<T>) : ptr_(std::move(u)) {
			if (ptr_ == nullptr) throw std::runtime_error{ "not_null object was assigned to nullptr" };
		}

		template<std::convertible_to<T> U>
		constexpr not_null(const not_null<U>& other) noexcept(std::is_nothrow_move_constructible_v<T>) : not_null(other.get()) {}

		not_null(const not_null& other) = default;
		not_null& operator=(const not_null& other) = default;

		constexpr inline value_or_reference_return_t<T> get() const noexcept(noexcept(value_or_reference_return_t<T>{ std::declval<T&>() })) {
			return ptr_;
		}
		constexpr operator T() const { return get(); }
		constexpr decltype(auto) operator->() const { return get(); }
		constexpr decltype(auto) operator*() const { return *get(); }

		// prevents compilation when someone attempts to assign a null pointer constant
		not_null(std::nullptr_t) = delete;
		not_null& operator=(std::nullptr_t) = delete;

		// unwanted operators...pointers only point to single objects!
		not_null& operator++() = delete;
		not_null& operator--() = delete;
		not_null operator++(int) = delete;
		not_null operator--(int) = delete;
		not_null& operator+=(std::ptrdiff_t) = delete;
		not_null& operator-=(std::ptrdiff_t) = delete;
		void operator[](std::ptrdiff_t) const = delete;

	private:
		T ptr_;
	};

	template<typename T>
	using shared_ref = not_null<std::shared_ptr<T>>;

	/** An array that is indexed with a enum class type, typically with a size that matches the number of elements in the enum */
	template<typename ValueType, enumeration EnumType, size_t Size = static_cast<size_t>(EnumType::MAX)>
	struct enum_array {
	public:
		enum_array() { array.fill(ValueType{}); }
		enum_array(EnumType value) { array.fill(static_cast<EnumType>(value)); }

		ValueType& operator[](EnumType value) { return array[std::to_underlying(value)]; }
		ValueType const& operator[](EnumType value) const { return array[std::to_underlying(value)]; }

		uint32_t size() const { return static_cast<uint32_t>(EnumType::MAX); }
		ValueType const* data() const { return array.data(); }

	private:
		std::array<ValueType, Size> array;
	};

	/**
	 * A value that can only be moved from the owning struct, not copied. When moved, the old variable will be set to the default-initialized value.
	 * The underlying value is not required to be move-only, and can be copied when accessed directly.
	 * The primary use of this is to ensure that a struct uses move semantics for ownership-related variables when using the default move constructor.
	 */
	template<std::semiregular T>
	struct move_only {
		explicit move_only(T const& value) : value(value) {}
		explicit move_only(T&& value) : value(value) {}

		move_only(move_only const&) = delete;
		move_only(move_only&& other) : value(other.value) { other.value = T{}; }

		move_only& operator=(move_only&& other) {
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

	template<typename T>
	size_t get_remaining(std::basic_istream<T>& stream) {
		auto const position = stream.tellg();
		stream.ignore(std::numeric_limits<std::streamsize>::max());
		auto const remaining = stream.gcount();
		stream.clear();
		stream.seekg(position);
		return remaining;
	}

	template<typename T>
	size_t get_total(std::basic_istream<T>& stream) {
		auto const position = stream.tellg();
		stream.seekg(std::ios::beg);
		stream.ignore(std::numeric_limits<std::streamsize>::max());
		auto const total = stream.gcount();
		stream.clear();
		stream.seekg(position);
		return total;
	}
}

//============================================================
// GLM types

#define GLM_FORCE_RADIANS
#define GLM_FORCE_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_aligned.hpp>
#include <glm/gtx/exterior_product.hpp>
#include <glm/matrix.hpp>
