#pragma once
#include <stdexcept>
#include "Engine/Concepts.h"
#include "Engine/Array.h"
#include "Engine/Core.h"

namespace Archive {
	/** Wraps a dynamic array of bytes in memory, and allows new objects to be encoded and added to those bytes. Similar to an ostream, but much simpler. */
	struct Output {
		using BufferType = std::vector<std::byte>;

		Output(BufferType& buffer) noexcept : buffer(buffer) {}

		Output(Output const&) noexcept = default;
		Output(Output&&) noexcept = default;
		Output& operator=(Output const&) noexcept = default;
		Output& operator=(Output&&) noexcept = default;

		inline size_t Size() const { return buffer.size(); }

	private:
		friend void WriteBytes(Output&, std::span<std::byte const>);

		BufferType& buffer;
	};

	/** References a fixed array of bytes in memory, and allows those bytes to be decoded into objects. Similar to an istream, but much simpler. */
	struct Input {
		Input(std::span<std::byte const> buffer) noexcept : buffer(buffer) {}
		Input(std::span<char const> chars) noexcept : buffer(reinterpret_cast<std::byte const*>(chars.data()), chars.size()) {}

		Input(Input const&) noexcept = default;
		Input(Input&&) noexcept = default;
		Input& operator=(Input const&) noexcept = default;
		Input& operator=(Input&&) noexcept = default;

		inline size_t Remaining() const noexcept { return buffer.size(); }

	private:
		friend std::span<std::byte const> ReadBytes(Input&, size_t);
		friend void Skip(Input&, size_t);
		friend Input Subset(Input const&, size_t);

		std::span<std::byte const> buffer;
	};
}
