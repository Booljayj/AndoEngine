#include <ft2build.h>
#include FT_OUTLINE_H
#include "Rendering/Font/FontUtility.h"
#include "Engine/LinearContainers.h"
#include "Engine/Context.h"
#include "Geometry/Contour.h"
#include "Geometry/Curve.h"

namespace Rendering {
	struct GlyphShapeContext {
		Geometry::Shape* Shape;
		Geometry::Contour* CurrentContour;
		glm::vec2 CurrentStartPosition;

		GlyphShapeContext( Geometry::Shape& InShape )
		: Shape( &InShape )
		, CurrentContour( nullptr )
		, CurrentStartPosition( 0.0f, 0.0f )
		{}
	};

	int32_t FreetypeMoveTo( FT_Vector const* End, void* ContextPtr ) {
		GlyphShapeContext& Context = *static_cast<GlyphShapeContext*>( ContextPtr );
		Context.Shape->Contours.push_back( Geometry::Contour{} );
		Context.CurrentContour = &Context.Shape->Contours.back();
		Context.CurrentStartPosition = glm::vec2{ End->x, End->y };
		return 0;
	}
	int32_t FreetypeLineTo( FT_Vector const* End, void* ContextPtr ) {
		GlyphShapeContext& Context = *static_cast<GlyphShapeContext*>( ContextPtr );
		glm::vec2 EndPosition{ End->x, End->y };
		Context.CurrentContour->Curves.push_back(
			Geometry::LinearCurve{ Context.CurrentStartPosition, EndPosition }
		);
		Context.CurrentStartPosition = EndPosition;
		return 0;
	}
	int32_t FreetypeConicTo( FT_Vector const* Ctl, FT_Vector const* End, void* ContextPtr ) {
		GlyphShapeContext& Context = *static_cast<GlyphShapeContext*>( ContextPtr );
		glm::vec2 EndPosition{ End->x, End->y };
		Context.CurrentContour->Curves.push_back(
			Geometry::QuadraticCurve{ Context.CurrentStartPosition, glm::vec2{ Ctl->x, Ctl->y }, EndPosition }
		);
		Context.CurrentStartPosition = EndPosition;
		return 0;
	}
	int32_t FreetypeCubicTo( FT_Vector const* Ctl0, FT_Vector const* Ctl1, FT_Vector const* End, void* ContextPtr ) {
		GlyphShapeContext& Context = *static_cast<GlyphShapeContext*>( ContextPtr );
		glm::vec2 EndPosition{ End->x, End->y };
		Context.CurrentContour->Curves.push_back(
			Geometry::CubicCurve{ Context.CurrentStartPosition, glm::vec2{ Ctl0->x, Ctl0->y }, glm::vec2{ Ctl1->x, Ctl1->y }, EndPosition }
		);
		Context.CurrentStartPosition = EndPosition;
		return 0;
	}

	FT_Outline_Funcs FreetypeOutlineFunctions{
		&FreetypeMoveTo,
		&FreetypeLineTo,
		&FreetypeConicTo,
		&FreetypeCubicTo,
		0, 0
	};

	FT_GlyphSlot LoadGlyph( FT_Face Face, char32_t CodePoint ) {
		FT_Error Error = FT_Load_Char( Face, CodePoint, FT_LOAD_NO_SCALE | FT_LOAD_PEDANTIC | FT_LOAD_LINEAR_DESIGN );
		if( Error ) return nullptr;
		else return Face->glyph;
	}

	float GetGlpyhAdvance( FT_GlyphSlot Glyph ) {
		return static_cast<float>( Glyph->metrics.horiAdvance );
	}

	bool DecomposeGlyph( Geometry::Shape& OutShape, FT_GlyphSlot Glyph ) {
		if( Glyph->format != FT_Glyph_Format_::FT_GLYPH_FORMAT_OUTLINE ) return false;
		//Reset the shape before writing to it
		OutShape.Contours.clear();
		//Decompose the shape of the glyph
		GlyphShapeContext Context{ OutShape };
		const FT_Error DecomposeError = FT_Outline_Decompose( &Glyph->outline, &FreetypeOutlineFunctions, &Context );
		if( DecomposeError ) return false;
		//Successfully decomposed the shape (shape may still be invalid if the decomposed data is corrupt)
		return true;
	}

	l_vector<::std::tuple<uint32_t, uint32_t, glm::vec2>> DumpKerningValues( CTX_ARG, FT_Face Face, char32_t CodePoint ) {
		l_vector<::std::tuple<uint32_t, uint32_t, glm::vec2>> KerningValues{ CTX.temp };
		if( FT_HAS_KERNING( Face ) ) {
			int32_t FirstGlyphIndex = FT_Get_Char_Index( Face, CodePoint );
			FT_Vector Kerning;
			for( size_t SecondGlyphIndex = 0; SecondGlyphIndex < Face->num_glyphs; ++SecondGlyphIndex ) {
				FT_Error Error = FT_Get_Kerning( Face, FirstGlyphIndex, SecondGlyphIndex, FT_Kerning_Mode_::FT_KERNING_UNSCALED, &Kerning );
				if( Error || ( Kerning.x == 0 && Kerning.y == 0 ) ) continue;
				KerningValues.push_back( std::make_tuple( FirstGlyphIndex, SecondGlyphIndex, glm::vec2{ Kerning.x, Kerning.y } ) );
			}
		}
		return KerningValues;
	}

}
