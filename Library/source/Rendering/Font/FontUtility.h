#pragma once
#include <tuple>
#include <glm/vec2.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Geometry/Shape.h"
#include "Engine/LinearContainers.h"

namespace Rendering {
	/** Load the glyph for a code point */
	FT_GlyphSlot LoadGlyph( FT_Face Face, char32_t CodePoint );

	/** Get the horizontal position change that should be applied after the glyph is placed */
	float GetGlyphAdvance( FT_GlyphSlot Glyph );

	/** Decompose a glyph outline into a shape object */
	bool DecomposeGlyph( Geometry::Shape& OutShape, FT_GlyphSlot Glyph );

	/** Find the kerning values for pairs that start with the code point */
	l_vector<std::tuple<uint32_t, uint32_t, glm::vec2>> DumpKerningValues( CTX_ARG, FT_Face Face, char32_t CodePoint );
}
