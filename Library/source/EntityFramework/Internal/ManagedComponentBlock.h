#pragma once
#include <array>
#include <bitset>
#include <cassert>

template< typename TCOMP, size_t BLOCK_SIZE >
struct TInlineManagedComponentBlock
{
	size_t LowestFreeIndex;
	std::bitset<BLOCK_SIZE> Used;
	std::array<TCOMP, BLOCK_SIZE> Data;

	TInlineManagedComponentBlock()
	: LowestFreeIndex( 0 )
	{}

	size_t CountUsed() const { return Used.count(); }
	bool Contains( TCOMP const* Value ) const { return Value >= Data.begin() && Value < Data.end(); }
	bool HasAnyFree() const { return LowestFreeIndex < BLOCK_SIZE; }
	size_t IndexOf( TCOMP const* Value ) const { return ( Value - Data.begin() ); }
	bool IsUsed( TCOMP const* Value ) const { return Used[IndexOf(Value)]; }

	TCOMP* Retain() {
		assert( HasAnyFree() );
		TCOMP* Retained = &Data[LowestFreeIndex];
		Used.set( LowestFreeIndex );
		do { ++LowestFreeIndex; }
		while( LowestFreeIndex < BLOCK_SIZE && Used.test( LowestFreeIndex ) );
		return Retained;
	}

	void Release( TCOMP* Released ) {
		assert( Contains( Released ) );
		size_t ReleasedIndex = IndexOf( Released );
		Used.reset( ReleasedIndex );
		if( ReleasedIndex < LowestFreeIndex ) LowestFreeIndex = ReleasedIndex;
	}

	size_t size() const { return Data.size(); }
	typename std::array<TCOMP, BLOCK_SIZE>::const_iterator begin() const { return Data.begin(); }
	typename std::array<TCOMP, BLOCK_SIZE>::const_iterator end() const { return Data.end(); }
	typename std::array<TCOMP, BLOCK_SIZE>::iterator begin() { return Data.begin(); }
	typename std::array<TCOMP, BLOCK_SIZE>::iterator end() { return Data.end(); }
};

template< typename TCOMP >
struct THeapManagedComponentBlock
{
	//Wrapper so ranged-based for loops will work with pointers
	struct Iterator
	{
		Iterator( TCOMP* InPtr ) : Ptr( InPtr ) {}
		void operator++() { ++Ptr; }
		bool operator!=( Iterator const& Other ) const { return Ptr != Other.Ptr; }
		TCOMP* operator*() const { return Ptr; }

	private:
		TCOMP* Ptr;
	};

	TCOMP* Data = nullptr;
	TCOMP* Capacity = nullptr;
	TCOMP* LowestFree = nullptr;
	uint8_t* UsedBytes = nullptr;
	size_t UsedCount = 0;

	THeapManagedComponentBlock( size_t BlockSize )
	{
		assert( BlockSize > 0 );
		Data = static_cast<TCOMP*>( std::malloc( sizeof( TCOMP ) * BlockSize ) );
		Capacity = Data + BlockSize;
		LowestFree = Data;

		//You would normally add one only to catch the remainder, but why not just do it every time?
		size_t NumUsageTrackingBytes = ( BlockSize / 8 ) + 1;
		UsedBytes = static_cast<uint8_t*>( std::malloc( NumUsageTrackingBytes ) );
		assert( Data != nullptr );
	}

	~THeapManagedComponentBlock()
	{
		std::free( static_cast<void*>( Data ) );
		std::free( static_cast<void*>( UsedBytes ) );
	}

	size_t CountUsed() const { return UsedCount; }
	bool Contains( TCOMP const* Value ) const { return Value >= Data && Value < Capacity; }
	bool HasAnyFree() const { return LowestFree < Capacity; }
	size_t IndexOf( TCOMP const* Value ) const { return Value - Data; }
	bool IsUsed( TCOMP const* Value ) const
	{
		const size_t Index = IndexOf( Value );
		return TEST_BIT( *( UsedBytes + ( Index / 8 ) ), ( 1 << ( Index / 8 ) ) );
	}
	void SetUsed( TCOMP const* Value )
	{
		++UsedCount;
		const size_t Index = IndexOf( Value );
		SET_BIT( *( UsedBytes + ( Index / 8 ) ), ( 1 << ( Index / 8 ) ) );
	}
	void SetFree( TCOMP const* Value )
	{
		--UsedCount;
		const size_t Index = IndexOf( Value );
		CLEAR_BIT( *( UsedBytes + ( Index / 8 ) ), ( 1 << ( Index / 8 ) ) );
	}

	TCOMP* Retain()
	{
		assert( HasAnyFree() );
		TCOMP* Retained = LowestFree;
		SetUsed( Retained );
		while( LowestFree < Capacity && IsUsed( LowestFree ) )
		{
			++LowestFree;
		}
		return Retained;
	}

	void Release( TCOMP* Released )
	{
		assert( Contains( Released ) );
		SetFree( Released );
		if( Released < LowestFree )
		{
			LowestFree = Released;
		}
	}

	Iterator begin() const { return Iterator{ Data }; }
	Iterator end() const { return Iterator{ Capacity }; }
};
