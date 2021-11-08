#include "Rendering/Font/FontUtility.h"
#include "Geometry/Contour.h"
#include "Geometry/Curve.h"

namespace Rendering {
	struct GlyphShapeContext {
		Geometry::Shape* shape;
		Geometry::Contour* currentContour;
		glm::vec2 currentStartPosition;

		GlyphShapeContext(Geometry::Shape& inShape)
		: shape(&inShape)
		, currentContour(nullptr)
		, currentStartPosition(0.0f, 0.0f)
		{}
	};

	int32_t FreetypeMoveTo(FT_Vector const* end, void* contextPtr) {
		GlyphShapeContext& context = *static_cast<GlyphShapeContext*>(contextPtr);
		context.shape->contours.push_back(Geometry::Contour{});
		context.currentContour = &context.shape->contours.back();
		context.currentStartPosition = glm::vec2{end->x, end->y};
		return 0;
	}
	int32_t FreetypeLineTo(FT_Vector const* end, void* contextPtr) {
		GlyphShapeContext& context = *static_cast<GlyphShapeContext*>(contextPtr);
		glm::vec2 endPosition{end->x, end->y};
		context.currentContour->curves.push_back(
			Geometry::LinearCurve{context.currentStartPosition, endPosition}
		);
		context.currentStartPosition = endPosition;
		return 0;
	}
	int32_t FreetypeConicTo(FT_Vector const* ctl, FT_Vector const* end, void* contextPtr) {
		GlyphShapeContext& context = *static_cast<GlyphShapeContext*>(contextPtr);
		glm::vec2 endPosition{end->x, end->y};
		context.currentContour->curves.push_back(
			Geometry::QuadraticCurve{context.currentStartPosition, glm::vec2{ctl->x, ctl->y}, endPosition}
		);
		context.currentStartPosition = endPosition;
		return 0;
	}
	int32_t FreetypeCubicTo(FT_Vector const* ctl0, FT_Vector const* ctl1, FT_Vector const* end, void* contextPtr) {
		GlyphShapeContext& context = *static_cast<GlyphShapeContext*>(contextPtr);
		glm::vec2 endPosition{end->x, end->y};
		context.currentContour->curves.push_back(
			Geometry::CubicCurve{context.currentStartPosition, glm::vec2{ctl0->x, ctl0->y}, glm::vec2{ctl1->x, ctl1->y}, endPosition}
		);
		context.currentStartPosition = endPosition;
		return 0;
	}

	FT_Outline_Funcs freetypeOutlineFunctions{
		&FreetypeMoveTo,
		&FreetypeLineTo,
		&FreetypeConicTo,
		&FreetypeCubicTo,
		0, 0
	};

	FT_GlyphSlot LoadGlyph(FT_Face face, char32_t codePoint) {
		FT_Error const error = FT_Load_Char(face, codePoint, FT_LOAD_NO_SCALE | FT_LOAD_PEDANTIC | FT_LOAD_LINEAR_DESIGN);
		if (error) return nullptr;
		else return face->glyph;
	}

	float GetGlpyhAdvance(FT_GlyphSlot glyph) {
		return static_cast<float>(glyph->metrics.horiAdvance);
	}

	bool DecomposeGlyph(Geometry::Shape& outShape, FT_GlyphSlot glyph) {
		if (glyph->format != FT_Glyph_Format_::FT_GLYPH_FORMAT_OUTLINE) return false;
		//Reset the shape before writing to it
		outShape.contours.clear();
		//Decompose the shape of the glyph
		GlyphShapeContext context{outShape};
		FT_Error const error = FT_Outline_Decompose(&glyph->outline, &freetypeOutlineFunctions, &context);
		if (error) return false;
		//Successfully decomposed the shape (shape may still be invalid if the decomposed data is corrupt)
		return true;
	}

	t_vector<::std::tuple<uint32_t, uint32_t, glm::vec2>> DumpKerningValues(FT_Face face, char32_t codePoint) {
		t_vector<::std::tuple<uint32_t, uint32_t, glm::vec2>> kerningValues;
		kerningValues.reserve(128);
		if (FT_HAS_KERNING(face)) {
			int32_t const firstGlyphIndex = FT_Get_Char_Index(face, codePoint);
			FT_Vector kerning;
			for (size_t secondGlyphIndex = 0; secondGlyphIndex < face->num_glyphs; ++secondGlyphIndex) {
				FT_Error const error = FT_Get_Kerning(face, firstGlyphIndex, secondGlyphIndex, FT_Kerning_Mode_::FT_KERNING_UNSCALED, &kerning);
				if (error || (kerning.x == 0 && kerning.y == 0)) continue;
				kerningValues.push_back(std::make_tuple(firstGlyphIndex, secondGlyphIndex, glm::vec2{kerning.x, kerning.y}));
			}
		}
		return kerningValues;
	}

}
