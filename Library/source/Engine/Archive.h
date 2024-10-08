#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"
#include "Engine/Utility.h"

namespace Archive {
	/** Contains a dynamic array of bytes in memory, and allows new objects to be encoded and added to those bytes. Similar to an ostream, but much simpler. */
	struct Output {
		using BufferType = std::vector<std::byte>;

		Output(BufferType& buffer) noexcept : buffer(buffer) {}

		Output(Output const&) noexcept = default;
		Output(Output&&) noexcept = default;
		Output& operator=(Output const&) noexcept = default;
		Output& operator=(Output&&) noexcept = default;

		inline size_t Size() const { return buffer.size(); }

		/** Write a span of bytes into the archive */
		template<size_t N>
		inline void Write(std::span<std::byte const, N> source) { stdext::append(buffer, source); }

	private:
		BufferType& buffer;
	};

	/** References a fixed array of bytes in memory, and allows those bytes to be decoded into objects. Similar to an istream, but much simpler. */
	struct Input {
		using ByteSpanType = std::span<std::byte const>;

		Input(ByteSpanType bytes) noexcept : bytes(bytes) {}
		Input(std::span<char const> chars) noexcept : bytes(reinterpret_cast<std::byte const*>(chars.data()), chars.size()) {}

		Input(Input const&) noexcept = default;
		Input(Input&&) noexcept = default;
		Input& operator=(Input const&) noexcept = default;
		Input& operator=(Input&&) noexcept = default;

		inline size_t Capacity() const noexcept { return bytes.size(); }
		inline size_t NumRead() const noexcept { return cursor; }
		inline size_t Remaining() const noexcept { return bytes.size() - cursor; }

		/** Skip past the specified number of bytes without reading them. Throws if the archive does not contain the requested number of bytes. */
		inline void Skip(size_t num) {
			VerifyRemaining(num);
			cursor += num;
		}

		/** Read the specified number of bytes from the archive, and return a span of them. Throws if the archive does not contain the requested number of bytes. */
		inline ByteSpanType Read(size_t num) {
			VerifyRemaining(num);
			size_t const offset = cursor;
			cursor += num;
			return ByteSpanType{ bytes.data() + offset, num };
		}
		/** Read the specified number of bytes from the archive, and return a span of them. Throws if the archive does not contain the requested number of bytes. */
		template<size_t N>
		inline std::span<std::byte const, N> Read() {
			VerifyRemaining(N);
			size_t const offset = cursor;
			cursor += N;
			return std::span<std::byte const, N>{ bytes.data() + offset, N };
		}

		/** Create a new archive that will read a subset of this archive, containing the provided number of bytes. Does not modify the source archive. Throws if the archive does not contain the requested number of bytes */
		Input Subset(size_t num) const {
			VerifyRemaining(num);
			std::byte const* begin = bytes.data() + cursor;
			return Input{ ByteSpanType{ begin, num } };
		}
		/** Create a new archive that will read a subset of this archive, containing all remaining bytes in this archive. Does not modify the source archive. */
		inline Input Subset() const noexcept {
			return Input{ ByteSpanType{ bytes.data() + cursor, Remaining() } };
		}

	private:
		ByteSpanType bytes;
		size_t cursor = 0;

		void VerifyRemaining(size_t num) const {
			if (Remaining() < num) throw FormatType<std::overflow_error>("Cannot read {} bytes from archive, this exceeds the size of the archive", num);
		}
	};

	//=============================================================================
	// Generic serialization support

	/** Implemented by types that support serializing or deserializing from an archive. Support may not be bi-directional. */
	template<typename T>
	struct Serializer {
		//void Write(Output& archive, T const& value);
		//void Read(Input& archive, T& value);
	};

	namespace Concepts {
		template<typename T>
		concept Writable = requires (T const& t, Output& archive) {
			{ Serializer<T>::Write(archive, t) };
		};
		template<typename T>
		concept Readable = requires (T & t, Input& archive) {
			{ Serializer<T>::Read(archive, t) };
		};
		template<typename T>
		concept ReadWritable = Writable<T> and Readable<T>;

		template<typename T, typename E>
		concept RangeOf = ranges::sized_range<T> && std::same_as<ranges::range_reference_t<T>, E>;

		template<typename T, typename E>
		concept ReadableRangeOf = RangeOf<T, E> && requires (T& t, size_t integer) {
			{ t.resize(integer) };
			{ t.operator[](integer) };
		};
	}

	//=============================================================================
	// Primitive types

	template<stdext::numeric T>
	struct Serializer<T> {
		static void Write(Output& archive, T const value) {
			if constexpr ((sizeof(T) == 1) || (boost::endian::order::native == boost::endian::order::little)) {
				const auto source = std::as_bytes(std::span<T const, 1>{ &value, 1 });
				archive.Write(source);
			} else {
				std::array<std::byte, sizeof(T)> temp;
				Utility::SaveOrdered(value, temp);
				archive.Write(temp);
			}
		}
		static void Read(Input& archive, T& value) {
			Utility::LoadOrdered(archive.Read<sizeof(T)>(), value);
		}
	};

	template<>
	struct Serializer<bool> {
		static void Write(Output& archive, bool const boolean) {
			//sizeof(bool) is implementation-defined, we'll ensure we only serialize a single byte regardless of the platform.
			uint8_t const value = static_cast<uint8_t>(boolean);
			Serializer<uint8_t>::Write(archive, value);
		}
		static void Read(Input& archive, bool& boolean) {
			//sizeof(bool) is implementation-defined, we'll ensure we only serialize a single byte regardless of the platform.
			uint8_t value = 0;
			Serializer<uint8_t>::Read(archive, value);
			boolean = static_cast<bool>(value);
		}
	};

	template<>
	struct Serializer<std::byte> {
		static void Write(Output& archive, std::byte const byte) {
			archive.Write(std::span{ &byte, 1 });
		}
		static void Read(Input& archive, std::byte& byte) {
			auto const bytes = archive.Read<1>();
			byte = bytes[0];
		}
	};
	template<>
	struct Serializer<std::byte const> {
		static void Write(Output& archive, std::byte const byte) {
			archive.Write(std::span{ &byte, 1 });
		}
	};

	//=============================================================================
	// String types

	template<stdext::character T>
	struct Serializer<std::basic_string<T>> {
		static void Write(Output& archive, std::basic_string<T> const& string) {
			Serializer<size_t>::Write(archive, string.size());
			for (T const character : string) Serializer<T>::Write(archive, character);
		}
		static void Read(Input& archive, std::basic_string<T>& string) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);
			
			string.resize(num);
			for (T& character : string) Serializer<T>::Read(archive, character);
		}
	};

	template<stdext::character T>
	struct Serializer<std::basic_string_view<T>> {
		static void Write(Output& archive, std::basic_string_view<T> const& string) {
			Serializer<size_t>::Write(archive, string.size());
			for (T const character : string) Serializer<T>::Write(archive, character);
		}
		static void Read(Input& archive, std::basic_string_view<T>& string) {
			size_t num = 0;
			Serializer<size_t>::Read(archive, num);

			std::span<std::byte const> const bytes = archive.Read(num * sizeof(T));
			string = std::basic_string_view<T>{ reinterpret_cast<T const*>(bytes.data()), num };
		}
	};

	//=============================================================================
	// Range types

	template<typename T>
	concept ReadWritableDynamicRange =
		ranges::sized_range<T> &&
		ranges::forward_range<T> &&
		Concepts::ReadWritable<ranges::range_value_t<T>> &&
		requires (T& t) {
			{ t.clear() };
			{ std::inserter<T> };
		};

	template<ReadWritableDynamicRange RangeType>
	struct Serializer<RangeType> {
		using ElementType = ranges::range_value_t<RangeType>;

		static void Write(Output& archive, RangeType const& range) {
			size_t const num = range.size();
			Serializer<size_t>::Write(archive, num);

			if constexpr (std::is_same_v<std::remove_const_t<ElementType>, std::byte>) {
				archive.Write(std::span<std::byte const>{ range });
			} else {
				for (const ElementType& element : range) Serializer<ElementType>::Write(archive, element);
			}
		}

		static void Read(Input& archive, RangeType& range) {
			range.clear();

			size_t num = 0;
			Serializer<size_t>::Read(archive, num);
			
			if constexpr (std::is_same_v<std::remove_const_t<ElementType>, std::byte>) {
				auto const span = archive.Read(num);
				auto inserter = std::inserter(range, range.end());
				for (size_t index = 0; index < num; ++index) inserter = span[index];
			} else {
				auto inserter = std::inserter(range, range.end());

				for (size_t index = 0; index < num; ++index) {
					ElementType element;
					Serializer<ElementType>::Read(archive, element);

					inserter = std::move(element);
				}
			}
		}
	};

	template<Concepts::ReadWritable T, size_t N>
	struct Serializer<std::array<T, N>> {
		static void Write(Output& archive, std::array<T, N> const& container) {
			if constexpr (std::is_same_v<std::remove_const_t<T>, std::byte>) {
				archive.Write(std::span<std::byte const>{ container });
			} else {
				for (const T& element : container) Serializer<T>::Write(archive, element);
			}
		}
		static void Read(Input& archive, std::array<T, N>& container) {
			if constexpr (std::is_same_v<std::remove_const_t<T>, std::byte>) {
				auto const span = archive.Read<N>();
				for (size_t index = 0; index < N; ++index) container[index] = span[index];
			} else {
				for (const T& element : container) Serializer<T>::Read(archive, element);
			}
		}
	};

	template<Concepts::Writable T, size_t Extent>
	struct Serializer<std::span<T, Extent>> {
		static void Write(Output& archive, std::span<T, Extent> const& span) {
			if constexpr (Extent == std::dynamic_extent) {
				size_t const num = span.size();
				Serializer<size_t>::Write(archive, num);
			}

			if constexpr (std::is_same_v<std::remove_const_t<T>, std::byte>) {
				archive.Write(span);
			} else {
				for (const T& element : span) Serializer<T>::Write(archive, element);
			}
		}
		static void Read(Input& archive, std::span<T, Extent>& span) {
			static_assert(std::is_trivial_v<T> && std::is_const_v<T>, "Only constant trivial types can be read from an archive as a span");

			if constexpr (Extent == std::dynamic_extent) {
				size_t num = 0;
				Serializer<size_t>::Read(archive, num);
				span = stdext::from_bytes<T>(archive.Read(num * sizeof(T)));
			} else {
				span = stdext::from_bytes<T>(archive.Read<Extent * sizeof(T)>());
			}
		}
	};
}

template<Archive::Concepts::Writable T>
Archive::Output& operator<<(Archive::Output& archive, T const& value) {
	Archive::Serializer<T>::Write(archive, value);
	return archive;
}

template<Archive::Concepts::Readable T>
Archive::Input& operator>>(Archive::Input& archive, T& value) {
	Archive::Serializer<T>::Read(archive, value);
	return archive;
}
