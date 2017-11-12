// Copyright © 2017 Justin Bool. All rights reserved.

#pragma once

#include <cassert>
#include <functional>
#include <algorithm>
#include <array>
#include <vector>
#include <bitset>
using namespace std;

#include "EntityFrameworkTypes.h"
#include "Serializer.h"

struct Entity;

struct ComponentManager
{
public:
	virtual ~ComponentManager() {}

	virtual bool Initialize() { return true; }
	virtual bool Deinitialize() { return true; }
	virtual void Setup( const Entity& NewEntity, raw_ptr NewComponent ) {}

	virtual raw_ptr Retain() = 0;
	virtual void Release( raw_ptr ) = 0;

	virtual size_t CountTotal() const = 0;
	virtual size_t CountFree() const = 0;
	virtual size_t CountUsed() const = 0;

	virtual void Save( const raw_ptr, ByteStream& ) = 0;
	virtual void Load( raw_ptr, const ByteStream& ) = 0;
	virtual void Copy( const raw_ptr, raw_ptr ) = 0;
};

template< typename TCOMP, size_t BLOCK_SIZE >
struct ManagedComponentBlock
{
	size_t LowestFreeIndex;
	bitset<BLOCK_SIZE> Free;
	array<TCOMP, BLOCK_SIZE> Data;

	ManagedComponentBlock()
		: LowestFreeIndex( 0 )
	{
		Free.set(); //start all true (free)
	}

	bool Contains( TCOMP* ValuePtr ) const { return ValuePtr >= Data.begin() && ValuePtr < Data.end(); }
	bool HasAnyFree() const { return Free.any(); }

	TCOMP* Retain()
	{
		assert( HasAnyFree() && LowestFreeIndex < BLOCK_SIZE );
		TCOMP* RetainedElement = Data.begin() + LowestFreeIndex;
		Free[LowestFreeIndex] = false;
		while( LowestFreeIndex < BLOCK_SIZE && !Free[LowestFreeIndex] )
		{
			++LowestFreeIndex;
		}
		return RetainedElement;
	}

	void Release( TCOMP* ElementPtr )
	{
		assert( Contains( ElementPtr ) );
		size_t ReleasedIndex = ElementPtr - Data.begin();
		Free.set( ReleasedIndex, true );
		if( ReleasedIndex < LowestFreeIndex )
		{
			LowestFreeIndex = ReleasedIndex;
		}
	}

	template< typename TPRED >
	void ForEach( TPRED& Pred )
	{
		for( size_t Index = 0; Index < BLOCK_SIZE; ++Index )
		{
			if( !Free[Index] ) Pred( &Data[Index] );
		}
	}

	template< typename TPRED >
	void ForEachAll( TPRED& Pred )
	{
		for( size_t Index = 0; Index < BLOCK_SIZE; ++Index )
		{
			Pred( &Data[Index], Free[Index] );
		}
	}
};

template< class TCOMP >
struct TComponentManager : public ComponentManager
{
	static constexpr const size_t BLOCK_SIZE = 64;

protected:
	size_t LowestFreeBlockIndex;
	vector<ManagedComponentBlock<TCOMP, BLOCK_SIZE>*> Blocks;

public:
	TCOMP* Cast( raw_ptr Comp ) { return static_cast<TCOMP*>( Comp ); }

	raw_ptr Retain() override final
	{
		//Grow the number of managed blocks if we don't currently have any free components
		if( LowestFreeBlockIndex == Blocks.size() )
		{
			Blocks.push_back( new ManagedComponentBlock<TCOMP, BLOCK_SIZE>() );
		}

		TCOMP* RetainedComponent = Blocks[LowestFreeBlockIndex]->Retain();

		//Seek ahead if we've run out of free components in the lowest block
		while( LowestFreeBlockIndex < Blocks.size() && !Blocks[LowestFreeBlockIndex]->HasAnyFree() )
		{
			++LowestFreeBlockIndex;
		}

		OnRetained( RetainedComponent );

		return RetainedComponent;
	}

	void Release( raw_ptr RawReleasedComponent ) override final
	{
		TCOMP* ReleasedComponent = Cast( RawReleasedComponent );
		for( size_t ContainingBlockIndex = 0; ContainingBlockIndex < Blocks.size(); ++ContainingBlockIndex )
		{
			auto* Block = Blocks[ContainingBlockIndex];
			if( Block->Contains( ReleasedComponent ) )
			{
				Block->Release( ReleasedComponent );
				if( ContainingBlockIndex < LowestFreeBlockIndex )
				{
					LowestFreeBlockIndex = ContainingBlockIndex;
				}
			}
		}
		assert( false ); //Should never reach this point unless the component was not part of this manager
	}

	size_t CountTotal() const override final
	{
		return BLOCK_SIZE * Blocks.size();
	}

	size_t CountFree() const override final
	{
		size_t RunningTotal = 0;
		for( size_t Index = 0; Index < Blocks.size(); ++Index )
		{
			RunningTotal += Blocks[Index]->Free.count();
		}
		return RunningTotal;
	}

	size_t CountUsed() const override
	{
		return CountTotal() - CountFree();
	}

	template< typename TPred, typename... TArgs >
	inline void ForEach( TPred& Pred, TArgs... Args )
	{
		for( size_t Index = 0; Index < Blocks.size(); ++Index )
		{
			Blocks[Index]->ForEach( Pred, Args... );
		}
	}

	template< typename TPred, typename... TArgs >
	inline void ForEachAll( TPred& Pred, TArgs... Args )
	{
		for( size_t Index = 0; Index < Blocks.size(); ++Index )
		{
			Blocks[Index]->ForEachAll( Pred, Args... );
		}
	}

	void Save( const raw_ptr Comp, ByteStream& Bytes ) override final { Serializer<TCOMP>::Save( *Cast( Comp ), Bytes ); }
	void Load( raw_ptr Comp, const ByteStream& Bytes ) override final { Serializer<TCOMP>::Load( *Cast( Comp ), Bytes ); }
	void Copy( const raw_ptr CompA, raw_ptr CompB ) override final { *Cast( CompB ) = *Cast( CompA ); }

protected:
	virtual void OnRetained( TCOMP* Comp ) {}
	virtual void OnReleased( TCOMP* Comp ) {}
};
