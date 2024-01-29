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

//Array types
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <queue>
#include <stack>
#include <vector>

//Map types
#include <map>
#include <unordered_map>

//Set types
#include <set>
#include <unordered_set>

//Utility types
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
#include <mutex>
#include <shared_mutex>
#include <thread>

//Localization
#include <locale>
#include <codecvt>

//Misc
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <numbers>
#include <numeric>
#include <ranges>
#include <source_location>
#include <span>
#include <typeinfo>
#include <utility>

//Third-party and upcoming
#include "ThirdParty/uuid.h"

namespace ranges = std::ranges;

//Custom extensions to the standard library
namespace stdext {
	/** A type that is an enum */
	template<typename T>
	concept enumeration = std::is_enum_v<T>;

	/** A type which can be compared to nullptr, but is not literally nullptr */
	template<typename T>
	concept null_comparable =
		std::equality_comparable_with<std::nullptr_t, T> and
		std::copyable<T> and
		!std::is_null_pointer_v<T>;

	constexpr inline size_t hash_combine(size_t a, size_t b) { return a ^ (b + 0x9e3779b9 + (a << 6) + (a >> 2)); }

	template<typename SourceType, std::constructible_from<SourceType const&> TargetType, typename TargetAllocatorType>
	constexpr void append(std::vector<TargetType, TargetAllocatorType>& target, std::span<SourceType const> const& source) {
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
