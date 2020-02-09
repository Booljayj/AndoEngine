#include "Engine/ScopedTempBlock.h"

ScopedTempBlock::ScopedTempBlock(CTX_ARG, char* inStartCursor)
	: cachedCTX(&CTX), startCursor(inStartCursor)
{}

ScopedTempBlock::~ScopedTempBlock() {
	Reset();
}

void ScopedTempBlock::Reset() const {
	cachedCTX->temp.SetCursor(startCursor);
}
