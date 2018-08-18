#pragma once
#include <vector>
#include <unordered_map>
#include <locale>
#include <codecvt>
#include "Engine/Context.h"
#include "Engine/LinearAllocator.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "EntityFramework/Types.h"
#include "Geometry/Rect.h"

struct TextureComponent;

struct GlyphInfo
{
	Geometry::Rect TextureRect; //uv coordinate rect inside the texture page where the glyph image can be found
	Geometry::Rect QuadRect; //rect defining the physical size of the glyph quad, in em
	float Advance; //How much space the glyph quad requires before the next glyph quad is placed
};

struct CodePointFrequencyTracker {
	std::unordered_map<char32_t, uint32_t> CodePointFrequencyMap;

	/** Add the contents of a string to the tracked code points */
	void TrackCodePoints( CTX_ARG, std::string_view String ) {
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t, TLinearAllocator<char32_t>, TLinearAllocator<char>> Converter{};
		l_u32string CodePointString = Converter.from_bytes( String.data(), String.data() + String.length() );
		for( char32_t CodePoint : CodePointString ) {
			++CodePointFrequencyMap[CodePoint];
		}
	}

	/** Returns a temporary array of all code points in this set, ordered with the most frequent code points first */
	l_vector<char32_t> GetOrderedCodePoints( CTX_ARG ) const {
		const size_t Count = CodePointFrequencyMap.size();
		const size_t StartingTempUsed = CTX.Temp.GetUsed();
		//Create a temporary array to hold all the pairs in the map
		l_vector<std::pair<char32_t, uint32_t>> Pairs{ CTX.Temp };
		Pairs.reserve( Count );
		for( const std::pair<char32_t, uint32_t>& CharacterFrequencyPair : CodePointFrequencyMap ) {
			Pairs.push_back( CharacterFrequencyPair );
		}
		//Sort the pairs descending by frequency, then ascending by code point
		std::sort(
			Pairs.begin(), Pairs.end(),
			[]( const auto& PairA, const auto& PairB ) {
				if( PairA.second != PairB.second ) return PairA.second > PairB.second;
				else return PairB.first < PairB.first;
			}
		);
		//Reset the temporary storage so that Pairs and Characters start at the same offset
		CTX.Temp.SetUsed( StartingTempUsed );
		l_vector<char32_t> Characters{ CTX.Temp };
		Characters.resize( Count );
		//Copy the first value of each pair to the return array. Since Pairs and Characters occupy the same memory but the
		// elements of Pairs are larger, we always read ahead of or at the write location.
		for( size_t Index = 0; Index < Count; ++Index ) {
			Characters[Index] = Pairs[Index].first;
		}
	}
};

struct FontFaceComponent
{
	//Information about each glyph contained in this font
	std::vector<GlyphInfo> GlyphInfos;
	//The mapping between a unicode character code and the index in the GlyphInfos array (always sorted by character code, no duplicates).
	std::vector<std::pair<char32_t, uint32_t>> CharacterMapping;
	//The kerning information for glyph pairs
	std::vector<std::tuple<char32_t, char32_t, float>> Kerning;

	EntityID PageEntity;
	TextureComponent* Page;
};

struct FontFamilyComponent
{
	//When text is defined, it can include tags which push font attribute states into the current rendering context.
	// Those attributes are used to find the best matching font face to use when rendering glyphs. If a glyph is not
	// found in the best matching font face, the next best is used, and so on.
	enum class FFontStyle : uint8_t {
		None		= 0,
		Italic		= 1 << 0,
		Bold		= 1 << 1,
		Light		= 1 << 2,
	};
	enum class EHanGlyphType : uint8_t {
		SimplifiedChinese = 0,
		TraditionalChinese,
		Japanese,
		Korean,
	};
	struct FontAttributes {
		FFontStyle Style;
		EHanGlyphType HanGlyphType;
	};

	FontFaceComponent* Default;
	std::vector<std::pair<FontAttributes, FontFaceComponent*>> Variants;
};
