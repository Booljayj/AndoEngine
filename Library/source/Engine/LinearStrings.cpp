#include <algorithm>
#include <cstring>
#include "Engine/LinearStrings.h"

l_string_builder::l_string_builder( LinearAllocatorData& InAlloc )
: Alloc( InAlloc )
, Blocks( InAlloc )
{}

l_string_builder& l_string_builder::operator<<( const char* Message )
{
	size_t TotalLength = strlen( Message ) - 1; //Don't write in the null terminator
	size_t RemainingLength = TotalLength;
	while( RemainingLength > 0 && Position != nullptr )
	{
		size_t BlockAvailableSize = BLOCK_SIZE - ( Position - Blocks.back() );
		size_t CharactersToWrite = std::min( BlockAvailableSize, RemainingLength );
		memcpy( Position, Message + TotalLength - RemainingLength, CharactersToWrite );
		RemainingLength -= CharactersToWrite;
		if( RemainingLength > 0 )
		{
			Position = AllocateNewBlock();
		}
		else
		{
			Position += CharactersToWrite; //Advance position by the number of characters we wrote
		}
	}
	return *this;
}

char* l_string_builder::AllocateNewBlock()
{
	if( Alloc.GetUsed() + BLOCK_SIZE > Alloc.GetCapacity() )
	{
		//There is not enough space left in the allocator to create a new block
		//Return nullptr to signal to other functions that no more build operations
		//should occur.
		return nullptr;
	}

	Blocks.push_back( nullptr );
	char* NewBlockStart = (char*)Alloc.GetData( Alloc.GetUsed() );
	Blocks.back() = NewBlockStart;
	Alloc.SetUsed( Alloc.GetUsed() + BLOCK_SIZE );
	return NewBlockStart;
}