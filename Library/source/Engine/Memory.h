#pragma once
#include <cstdint>
#include <cstdlib>

namespace
{
	template< typename T, typename... TARGS >
	uint8_t* JointAllocateRecursive( size_t prev_end_offset, T*& view_ptr, size_t size, TARGS&&... args )
	{
		const size_t alignment = alignof( T );
		const size_t next_start_offset = ( ( prev_end_offset + alignment - 1 ) / alignment ) * alignment;
		const size_t next_end_offset = next_start_offset + ( size * sizeof( T ) );

		char* memory = JointAllocateRecursive( next_end_offset, args... );
		view_ptr = reinterpret_cast<T*>( memory + next_start_offset );

		return memory;
	}

	inline uint8_t* JointAllocateRecursive( size_t final_end_offset )
	{
		return static_cast<uint8_t*>( std::malloc( final_end_offset ) );
	}
}

/** Create a set of fixed-size arrays that are contiguous in memory */
template< typename... TARGS >
uint8_t* JointAllocate( TARGS&&... args )
{
	return static_cast<void*>( JointAllocateRecursive( 0, args... ) );
}

//Invoked like so:
// T1* View1;
// const size_t DesiredSize1;
// T2* View2;
// const size_t DesiredSize2;
// uint8_t* MemPtr = joint_malloc( View1, DesiredSize1, View2, DesiredSize2 );
