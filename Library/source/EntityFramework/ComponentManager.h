#pragma once
#include <cassert>
#include <functional>
#include <algorithm>
#include <array>
#include <vector>
#include <bitset>
#include "EntityFramework/Types.h"
#include "EntityFramework/Serializer.h"

struct Entity;

/** Interface for a class which can manage the lifecycle and assignment of components */
struct ComponentManager
{
public:
	virtual ~ComponentManager() {}

	/** Initialize the component manager, returns true on success */
	virtual bool Initialize() { return true; }
	/** Deinitialize the component manager, returns true on success */
	virtual bool Deinitialize() { return true; }
	/** Set up an entity that has just been assigned a component */
	virtual void Setup( const Entity& NewEntity, ptr_t NewComponent ) {}

	/** Retain an instance of a component to be used by a specific entity, returning a pointer to  it */
	virtual ptr_t Retain() = 0;
	/** Release a component that was previously retained */
	virtual void Release( ptr_t ) = 0;

	/** Number of component instances that this manager has created */
	virtual size_t CountTotal() const = 0;
	/** Number of component instances that have not been assigned to an entity */
	virtual size_t CountFree() const = 0;
	/** Number of component instances that have been assigned to an entity */
	virtual size_t CountUsed() const = 0;

	/** Serialize the state of a component to a byte stream */
	virtual void Save( const ptr_t, ByteStream& ) = 0;
	/** Deserialize the state of a component from a byte stream */
	virtual void Load( ptr_t, const ByteStream& ) = 0;
	/** Duplicate a component's state */
	virtual void Copy( const ptr_t, ptr_t ) = 0;
};

template< typename TCOMP, size_t BLOCK_SIZE >
struct ManagedComponentBlock
{
	//@todo: these asserts are to prepare for a change where the size of a ManagedComponentBlock can be provided during initialization.
	static_assert( BLOCK_SIZE <= sizeof( size_t )*8, "Number of components per block should not exceed sizeof(size_t)*8" );
	static_assert( BLOCK_SIZE != 0, "Number of components per block cannot be 0" );

	size_t LowestFreeIndex;
	std::bitset<BLOCK_SIZE> Free;
	std::array<TCOMP, BLOCK_SIZE> Data;

	ManagedComponentBlock()
		: LowestFreeIndex( 0 )
	{
		//@todo: Change this to track used components, not free ones. It just seems cleaner.
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
	void ForEach( const TPRED& Pred )
	{
		for( size_t Index = 0; Index < BLOCK_SIZE; ++Index )
		{
			if( !Free[Index] ) Pred( &Data[Index] );
		}
	}

	template< typename TPRED >
	void ForEachAll( const TPRED& Pred )
	{
		for( size_t Index = 0; Index < BLOCK_SIZE; ++Index )
		{
			Pred( &Data[Index], Free[Index] );
		}
	}
};

/** A component manager for a type of component */
template< class TCOMP >
struct TComponentManager : public ComponentManager
{
	//@todo: Switch to a method that allows this to be set during initialization, along with the maximum number of blocks.
	static constexpr const size_t BLOCK_SIZE = 64;

protected:
	size_t LowestFreeBlockIndex;
	std::vector<ManagedComponentBlock<TCOMP, BLOCK_SIZE>*> Blocks;

public:
	/** Convenience function to cast a raw pointer to component pointer */
	TCOMP* Cast( ptr_t Comp ) { return static_cast<TCOMP*>( Comp ); }

	ptr_t Retain() override final
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

	void Release( ptr_t RawReleasedComponent ) override final
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

	//Iterate over all retained components in this manager, calling the predicate with the component as the first argument.
	template< typename TPred, typename... TArgs >
	inline void ForEach( const TPred& Pred )
	{
		for( size_t Index = 0; Index < Blocks.size(); ++Index )
		{
			Blocks[Index]->ForEach( Pred );
		}
	}

	//Iterate over all components in this manager, calling the predicate with the component as the first argument and the usage status as the second.
	template< typename TPred, typename... TArgs >
	inline void ForEachAll( const TPred& Pred )
	{
		for( size_t Index = 0; Index < Blocks.size(); ++Index )
		{
			Blocks[Index]->ForEachAll( Pred );
		}
	}

	void Save( const ptr_t Comp, ByteStream& Bytes ) override final { Serializer<TCOMP>::Save( *Cast( Comp ), Bytes ); }
	void Load( ptr_t Comp, const ByteStream& Bytes ) override final { Serializer<TCOMP>::Load( *Cast( Comp ), Bytes ); }
	void Copy( const ptr_t CompA, ptr_t CompB ) override final { *Cast( CompB ) = *Cast( CompA ); }

protected:
	virtual void OnRetained( TCOMP* Comp ) {}
	virtual void OnReleased( TCOMP* Comp ) {}
};
