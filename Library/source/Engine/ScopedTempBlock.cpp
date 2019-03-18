#include "Engine/ScopedTempBlock.h"

ScopedTempBlock::ScopedTempBlock( CTX_ARG, char* InStartCursor )
	: CTX_Cached( &CTX ), StartCursor( InStartCursor )
{}

ScopedTempBlock::~ScopedTempBlock() {
	Reset();
}

void ScopedTempBlock::Reset() const {
	CTX_Cached->Temp.SetCursor( StartCursor );
}
