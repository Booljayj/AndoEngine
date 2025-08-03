#pragma once
#include "Engine/Core.h"
#include "Engine/GLM.h"
#include "Engine/TemporaryContainers.h"
#include "Engine/Tuple.h"
#include "Geometry/Shape.h"
#include "Rendering/Font/FreeType.h"

namespace Rendering {
	/** Load the glyph for a code point */
	FT_GlyphSlot LoadGlyph(FT_Face face, char32_t codePoint);

	/** Get the horizontal position change that should be applied after the glyph is placed */
	float GetGlyphAdvance(FT_GlyphSlot glyph);

	/** Decompose a glyph outline into a shape object */
	bool DecomposeGlyph(Geometry::Shape& outShape, FT_GlyphSlot glyph);

	/** Find the kerning values for pairs that start with the code point */
	t_vector<std::tuple<uint32_t, uint32_t, glm::vec2>> DumpKerningValues(FT_Face face, char32_t codePoint);
}
