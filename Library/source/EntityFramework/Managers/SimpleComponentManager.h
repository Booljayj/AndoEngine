#pragma once
#include "EntityFramework/Managers/ChunkedComponentManager.h"

struct Entity;

/** An extremely basic component manager that does not perform any additional lifetime functions other than wiping newly retained components. */
template< typename TCOMP >
struct TSimpleComponentManager : TChunkedComponentManager<TSimpleComponentManager<TCOMP>, TCOMP, 64>
{
	using TChunkedComponentManager<TSimpleComponentManager, TCOMP, 64>::TChunkedComponentManager;

	inline void OnSetup( const Entity& NewEntity, TCOMP* NewComponent ) const {}
	inline void OnRetained( TCOMP* Comp ) { this->Wipe( Comp ); }
	inline void OnReleased( TCOMP* Comp ) {}
};
