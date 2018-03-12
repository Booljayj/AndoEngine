#pragma once
#include <cstdint>
#include <cstdlib>

namespace
{
	inline uint8_t* JointAllocateRecursive( size_t const FinalEndOffset )
	{
		return reinterpret_cast<uint8_t*>( std::malloc( FinalEndOffset ) );
	}

	template< typename T, typename... TARGS >
	inline uint8_t* JointAllocateRecursive( size_t const PrevEndOffset, T*& OutPointer, size_t const Size, TARGS&&... Args )
	{
		size_t const Alignment = alignof( T );
		size_t const CurStartOffset = ( ( PrevEndOffset + Alignment - 1 ) / Alignment ) * Alignment;
		size_t const CurEndOffset = CurStartOffset + ( Size * sizeof( T ) );

		uint8_t* const Buffer = JointAllocateRecursive( CurEndOffset, Args... );
		OutPointer = reinterpret_cast<T*>( Buffer + CurStartOffset );

		return Buffer;
	}
}

/**
 * Create a set of fixed-size arrays that are contiguous in memory.
 * Invoked like so: void* Buffer = JointAllocate( PtrA, SizeA, PtrB, SizeB, PtrC, SizeC );
 * DO NOT DEALLOCATE INDIVIDUAL ARRAYS. Only the returned void* should ever be freed.
 */
template< typename... TARGS >
void* JointAllocate( TARGS&&... Args )
{
	return reinterpret_cast<void*>( JointAllocateRecursive( 0, Args... ) );
}
