#pragma once
#include "Engine/Context.h"
#include "Engine/Flags.h"
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/STL.h"
#include "Geometry/Rect.h"

struct TextureComponent;

struct GlyphInfo {
	Geometry::Rect textureRect; //uv coordinate rect inside the texture page where the glyph image can be found
	Geometry::Rect quadRect; //rect defining the physical size of the glyph quad, in em
	float advance; //How much space the glyph quad requires before the next glyph quad is placed
};

struct CodePointFrequencyTracker {
	std::unordered_map<char32_t, uint32_t> codePointFrequencyMap;

	/** Add the contents of a string to the tracked code points */
	void TrackCodePoints(CTX_ARG, std::string_view string) {
		//@todo The standard code converting algorithms won't allow the use of the temporary allocator.
		// This is a no-go, we need to find an alternate method

		// std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t, TLinearAllocator<char32_t>> Converter{};
		// l_u32string CodePointString = Converter.from_bytes( String.data(), String.data() + String.length() );
		// for( char32_t CodePoint : CodePointString ) {
		// 	++codePointFrequencyMap[CodePoint];
		// }
	}

	/** Returns a temporary array of all code points in this set, ordered with the most frequent code points first */
	l_vector<char32_t> GetOrderedCodePoints(CTX_ARG) const {
		size_t const count = codePointFrequencyMap.size();
		char* const startingCurrent = CTX.temp.GetCursor();
		//Create a temporary array to hold all the pairs in the map
		l_vector<std::pair<char32_t, uint32_t>> pairs{CTX.temp};
		pairs.reserve(count);
		for( const std::pair<char32_t, uint32_t>& characterFrequencyPair : codePointFrequencyMap ) {
			pairs.push_back( characterFrequencyPair );
		}
		//Sort the pairs descending by frequency, then ascending by code point
		std::sort(
			pairs.begin(), pairs.end(),
			[]( const auto& pairA, const auto& pairB ) {
				if (pairA.second != pairB.second) return pairA.second > pairB.second;
				else return pairB.first < pairB.first;
			}
		);
		//Reset the temporary storage so that pairs and characters start at the same offset
		CTX.temp.SetCursor(startingCurrent);
		l_vector<char32_t> characters{CTX.temp};
		characters.resize(count);
		//Copy the first value of each pair to the return array. Since pairs and characters occupy the same memory but the
		// elements of pairs are larger, we always read ahead of or at the write location.
		for( size_t index = 0; index < count; ++index ) {
			characters[index] = pairs[index].first;
		}
		return characters;
	}
};

struct FontFaceComponent {
	//Information about each glyph contained in this font
	std::vector<GlyphInfo> glyphInfos;
	//The mapping between a unicode character code and the index in the glyphInfos array (always sorted by character code, no duplicates).
	std::vector<std::pair<char32_t, uint32_t>> characterMapping;
	//The kerning information for glyph pairs
	std::vector<std::tuple<char32_t, char32_t, float>> kerning;

	EntityID pageEntity;
	TextureComponent* page;
};

struct FontFamilyComponent {
	//When text is defined, it can include tags which push font attribute states into the current rendering context.
	// Those attributes are used to find the best matching font face to use when rendering glyphs. If a glyph is not
	// found in the best matching font face, the next best is used, and so on.
	enum class EFontStyle : uint8_t {
		Italic,
		Bold,
		Light,
	};
	using FFontStyle = TFlags<EFontStyle>;

	enum class EHanGlyphType : uint8_t {
		SimplifiedChinese = 0,
		TraditionalChinese,
		Japanese,
		Korean,
	};
	struct FontAttributes {
		FFontStyle style;
		EHanGlyphType hanGlyphType;
	};

	FontFaceComponent* defaultFontFace;
	std::vector<std::pair<FontAttributes, FontFaceComponent*>> variants;
};
