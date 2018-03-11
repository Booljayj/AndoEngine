#include "Engine/ScopedTempBlock.h"

ScopedTempBlock::ScopedTempBlock( CTX_ARG, size_t InMark )
	: CTX_Cached( CTX ), Mark( InMark )
{}

ScopedTempBlock::~ScopedTempBlock()
{
	Reset();
}

void ScopedTempBlock::Reset() const
{
	CTX_Cached.Temp.SetUsed( Mark );
}