//
//  storage.h
//  ECS
//
//  Created by Justin Bool on 4/8/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include <cassert>
#include <functional>
#include <algorithm>
#include <array>
#include <vector>
#include <bitset>
using namespace std;

#include "CompManager.h"
#include "Serializer.h"

constexpr size_t MANAGER_BLOCK_SIZE = 64;

template< class TTData >
struct TCompManager : public CompManager
{
protected:
	struct ManagedBlock
	{
		array<TTData, MANAGER_BLOCK_SIZE> Data;
		bitset<MANAGER_BLOCK_SIZE> Free;
		size_t LowestFreeIndex;

		ManagedBlock()
		: LowestFreeIndex( 0 )
		{
			Free.set(); //start all true (free)
		}

		bool Contains( TTData* ValuePtr ) const { return ValuePtr >= Data.begin() && ValuePtr < Data.end(); }
		bool HasAnyFree() const { return Free.any(); }

		TTData* Retain()
		{
			assert( HasAnyFree() && LowestFreeIndex < MANAGER_BLOCK_SIZE );
			TTData* RetainedElement = Data.begin() + LowestFreeIndex;
			Free[LowestFreeIndex] = false;
			++LowestFreeIndex;
			while( LowestFreeIndex < MANAGER_BLOCK_SIZE && !Free[LowestFreeIndex] )
			{
				++LowestFreeIndex;
			}
			return RetainedElement;
		}

		void Release( TTData* ElementPtr )
		{
			assert( Contains( ElementPtr ) );
			size_t ReleasedIndex = ElementPtr - Data.begin();
			Free.set( ReleasedIndex, true );
			if( ReleasedIndex < LowestFreeIndex )
			{
				LowestFreeIndex = ReleasedIndex;
			}
		}

		template< typename TPred, typename... TArgs >
		void ForEach( TPred& Pred, TArgs... Args )
		{
			for( size_t Index = 0; Index < MANAGER_BLOCK_SIZE; ++Index )
			{
				if( !Free[Index] )
				{
					Pred( &Data[Index], Args... );
				}
			}
		}
	};

	vector<ManagedBlock*> Blocks;
	size_t LowestFreeBlockIndex;

public:
	TTData* Cast( raw_ptr Comp ) { return static_cast<TTData*>( Comp ); }

	raw_ptr Retain() override
	{
		//Grow the number of managed blocks if we don't currently have any free components
		if( LowestFreeBlockIndex == Blocks.size() )
		{
			Blocks.push_back( new ManagedBlock() );
		}

		TTData* RetainedComponent = Blocks[LowestFreeBlockIndex]->Retain();

		//Seek ahead if we've run out of free components in the lowest block
		while( LowestFreeBlockIndex < Blocks.size() && !Blocks[LowestFreeBlockIndex]->HasAnyFree() )
		{
			++LowestFreeBlockIndex;
		}

		Retained.push_back( RetainedComponent );
		RetainedComponent->OnRetained();

		return RetainedComponent;
	}

	void Release( raw_ptr RawReleasedComponent ) override
	{
		//Released components are not fully released until the manager is flushed
		Released.push_back( RawReleasedComponent );
	}

	void Flush() override
	{
		size_t BlockCount = Blocks.size();
		size_t ContainingBlockIndex = 0;

		// Sort pointers by value ascending to increase the likelihood releasing all pointers in a given block at once
		std::sort( Released.begin(), Released.end() );

		for( raw_ptr RawReleasedComponent : Released )
		{
			TTData* ReleasedComponent = Cast( RawReleasedComponent );

			// Each time this loop is run, it iterates over the blocks starting at the last index and looping around until it gets to the previous (looping) index
			// This done to optimize cases where the released components are in a sequence within the same block
			size_t PreviousContainingBlockIndex = ContainingBlockIndex;
			do
			{
				if( Blocks[ContainingBlockIndex]->Contains( ReleasedComponent ) )
				{
					Blocks[ContainingBlockIndex]->Release( ReleasedComponent );
					ReleasedComponent->OnReleased();
					ReleasedComponent = nullptr; //Signals the loop to stop
				}
				else
				{
					//Check the next block, looping around to the first if we've reached the end of the array
					ContainingBlockIndex = ( ContainingBlockIndex + 1 ) % BlockCount;
				}
			}
			while( ContainingBlockIndex != PreviousContainingBlockIndex && ReleasedComponent != nullptr );

			if( ContainingBlockIndex < LowestFreeBlockIndex )
			{
				LowestFreeBlockIndex = ContainingBlockIndex;
			}

			assert( ReleasedComponent == nullptr ); //Should never happen unless the blocks don't contain the component
		}

		Retained.clear();
		Released.clear();
	}

	size_t CountTotal() const override
	{
		return MANAGER_BLOCK_SIZE * Blocks.size();
	}

	size_t CountFree() const override
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
	inline void ForEach( TPred&& Pred, TArgs... Args )
	{
		ForEach( Pred, Args... );
	}

	template< typename TPred, typename... TArgs >
	inline void ForEach( TPred& Pred, TArgs... Args )
	{
		for( size_t Index = 0; Index < Blocks.size(); ++Index )
		{
			Blocks[Index]->ForEach( Pred, Args... );
		}
	}

	void Save( const raw_ptr Comp, ByteStream& Bytes ) override { Serializer<TTData>::Save( *Cast( Comp ), Bytes ); }
	void Load( raw_ptr Comp, const ByteStream& Bytes ) override { Serializer<TTData>::Load( *Cast( Comp ), Bytes ); }
	void Copy( const raw_ptr CompA, raw_ptr CompB ) override { *Cast( CompB ) = *Cast( CompA ); }

protected:
	virtual void OnRetained( TTData* Comp ) {}
	virtual void OnReleased( TTData* Comp ) {}
};
