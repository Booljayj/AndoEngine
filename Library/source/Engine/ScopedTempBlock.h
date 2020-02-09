#pragma once
#include "Engine/Context.h"

//Scope object that sets the context temporary allocator to the provided mark when going out of scope. Should typically be used through the macro that auto-sets the original used level upon creation.
struct ScopedTempBlock {
	ScopedTempBlock(CTX_ARG, char* inStartCursor);
	~ScopedTempBlock();

	void Reset() const;

private:
	Context* cachedCTX;
	char* startCursor;
};

/** Used at the beginning of a scope to indicate that temporary allocations will be reset when the scope ends */
#define TEMP_SCOPE ScopedTempBlock const ScopedTempBlock_{ CTX, CTX.temp.GetCursor() }
/** Reset the temporary allocation scope early. All allocations made before this is called will become invalid. */
#define TEMP_SCOPE_RESET __ScopedTempBlock.Reset()
