#pragma once
#include <vector>
#include <array>
#include <bitset>
#include "Engine/UtilityMacros.h"
#include "EntityFramework/Internal/ManagedComponentBlock.h"
#include "EntityFramework/ComponentManager.h"
#include "EntityFramework/Serializer.h"

/** A component manager which allocates fixed-size component chunks dynamically as needed and uses standardized manipulation */
template< class TIMPL, class TCOMP, size_t BLOCK_SIZE>
struct TChunkedComponentManager : public ComponentManager
{
	using BlockType = TInlineManagedComponentBlock<TCOMP, BLOCK_SIZE>;

protected:
	size_t LowestFreeBlockIndex = 0;
	std::vector<BlockType*> Blocks;

public:
	TChunkedComponentManager() = default;
	~TChunkedComponentManager()
	{
		for( BlockType* Block : Blocks ) { std::free( static_cast<void*>( Block ) ); }
	}

	static TCOMP* Cast( ptr_t Comp ) { return static_cast<TCOMP*>( Comp ); }
	static TCOMP const* Cast( cptr_t Comp ) { return static_cast<TCOMP const*>( Comp ); }

	virtual void Setup( Entity const& NewEntity, ptr_t NewComponent ) const override
	{
		static_cast<TIMPL const*>( this )->OnSetup( NewEntity, Cast( NewComponent ) );
	}

	ptr_t Retain() override final
	{
		//Grow the number of managed blocks if we don't currently have any free components
		if( LowestFreeBlockIndex == Blocks.size() ) {
			Blocks.push_back( new BlockType() );
		}
		TCOMP* RetainedComponent = Blocks[LowestFreeBlockIndex]->Retain();

		//Seek ahead if we've run out of free components in the lowest block
		while( LowestFreeBlockIndex < Blocks.size() && !Blocks[LowestFreeBlockIndex]->HasAnyFree() ) {
			++LowestFreeBlockIndex;
		}

		static_cast<TIMPL*>( this )->OnRetained( RetainedComponent );
		return RetainedComponent;
	}

	void Release( ptr_t RawReleasedComponent ) override final
	{
		TCOMP* ReleasedComponent = Cast( RawReleasedComponent );
		static_cast<TIMPL*>( this )->OnReleased( ReleasedComponent );
		for( size_t ContainingBlockIndex = 0; ContainingBlockIndex < Blocks.size(); ++ContainingBlockIndex ) {
			BlockType* Block = Blocks[ContainingBlockIndex];
			if( Block->Contains( ReleasedComponent ) ) {
				Block->Release( ReleasedComponent );
				if( ContainingBlockIndex < LowestFreeBlockIndex ) {
					LowestFreeBlockIndex = ContainingBlockIndex;
				}
				return;
			}
		}
		assert( false ); //Should never reach this point unless the component was not part of this manager
	}

	virtual void Save( cptr_t Comp, ByteStream& Bytes ) override { Serializer<TCOMP>::Save( *Cast( Comp ), Bytes ); }
	virtual void Load( ptr_t Comp, ByteStream const& Bytes ) override { Serializer<TCOMP>::Load( *Cast( Comp ), Bytes ); }
	virtual void Copy( cptr_t CompA, ptr_t CompB ) override { *Cast( CompB ) = *Cast( CompA ); }
	virtual void Wipe( ptr_t Comp ) override { new (Comp) TCOMP{}; }

	size_t CountTotal() const override final
	{
		return BLOCK_SIZE * Blocks.size();
	}

	size_t CountFree() const override final
	{
		return CountTotal() - CountUsed();
	}

	size_t CountUsed() const override
	{
		size_t RunningTotal = 0;
		for( size_t Index = 0; Index < Blocks.size(); ++Index ) {
			RunningTotal += Blocks[Index]->CountUsed();
		}
		return RunningTotal;
	}

	template< typename TPRED >
	void ForEach( TPRED const& Predicate ) const
	{
		for( BlockType* Block : Blocks ) {
			for( TCOMP& Comp : *Block ) {
				if( Block->IsUsed( &Comp ) ) Predicate( &Comp );
			}
		}
	}

	template< typename TPRED >
	void ForEachAll( TPRED const& Predicate ) const {
		for( auto* Block : Blocks ) {
			for( TCOMP& Comp : *Block ) {
				Predicate( &Comp );
			}
		}
	}
};
