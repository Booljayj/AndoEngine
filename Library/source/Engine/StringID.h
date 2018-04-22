#pragma once
#include <cstddef>
#include <cstdint>

constexpr inline uint32_t operator ""_id( char const* Str, size_t Size )
{
	//DJB2 Algorithm
	uint32_t Ret = 5381;
	while( *Str ) {
		Ret = ( Ret * 33 ) + *Str;
		++Str;
	}
	return Ret;
}

constexpr inline uint32_t id( char const* Str )
{
	if( Str != nullptr ) {
		return operator""_id( Str, 0 );
	} else {
		return 0;
	}
}
