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


//Localization
#include <locale>
#include <codecvt>

//Files
#include <filesystem>
#include <source_location>


//Misc
#include <bit>
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


//Custom extensions to the standard library
namespace stdext {
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

	template<typename T>
	using function_reference = absl::FunctionRef<T>;

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
