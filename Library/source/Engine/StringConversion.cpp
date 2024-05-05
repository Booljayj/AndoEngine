#include "Engine/StringConversion.h"
#include <cuchar>

template<>
void ConvertString<std::string, std::u8string>(std::u8string const& f, std::string& t) {
	t.resize(f.size());
	for (size_t index = 0; index < f.size(); ++index) t[index] = static_cast<char>(f[index]);
}

template<>
void ConvertString<std::string, std::u16string>(std::u16string const& f, std::string& t) {
	t.reserve(f.size());

	std::mbstate_t state{}; // zero-initialized to initial state
	char buffer[MB_LEN_MAX];

	for (char16_t const c : f) {
		size_t const num = std::c16rtomb(buffer, c, &state);
		t.append(buffer, num);
	}
}

template<>
void ConvertString<std::string, std::u32string>(std::u32string const& f, std::string& t) {
	t.reserve(f.size());

	std::mbstate_t state{}; // zero-initialized to initial state
	char buffer[MB_LEN_MAX];

	for (char32_t const c : f) {
		size_t const num = std::c32rtomb(buffer, c, &state);
		t.append(buffer, num);
	}
}

template<>
void ConvertString<std::u8string, std::string>(std::string const& f, std::u8string& t) {
	t.resize(f.size());
	for (size_t index = 0; index < f.size(); ++index) t[index] = static_cast<char8_t>(f[index]);
}

template<>
void ConvertString<std::u16string, std::string>(std::string const& f, std::u16string& t) {
	std::mbstate_t state{}; // zero-initialized to initial state
	char16_t c16 = 0;
	const char* ptr = f.data();
	const char* end = f.data() + f.size();

	t.clear();
	t.reserve(f.size());

	while (std::size_t rc = std::mbrtoc16(&c16, ptr, end - ptr + 1, &state)) {
		if (rc == (size_t)-1) throw std::runtime_error{ "encoding error" };
		if (rc == (size_t)-2) throw std::runtime_error{ "incomplete multibyte sequence" };

		if (rc >= 0) {
			t.append(&c16, 1);
			ptr += rc;
		}
	}
}

template<>
void ConvertString<std::u32string, std::string>(std::string const& f, std::u32string& t) {
	std::mbstate_t state{}; // zero-initialized to initial state
	char32_t c32 = 0;
	const char* ptr = f.data();
	const char* end = f.data() + f.size();

	t.clear();
	t.reserve(f.size());

	while (std::size_t rc = std::mbrtoc32(&c32, ptr, end - ptr + 1, &state)) {
		if (rc == (size_t)-1) throw std::runtime_error{ "encoding error" };
		if (rc == (size_t)-2) throw std::runtime_error{ "incomplete multibyte sequence" };

		if (rc >= 0) {
			t.append(&c32, 1);
			ptr += rc;
		}
	}
}
