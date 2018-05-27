#pragma once
#include <cstddef>
#include <cstdint>

constexpr inline uint32_t operator ""_id( char const* Str, size_t Size )
{
	//DJB2 Algorithm
	uint32_t Ret = 5381;
	for( size_t Index = 0; Index < Size; ++Index, ++Str ) {
		Ret = ( Ret * 33 ) + *Str;
	}
	return Ret;
}

constexpr inline uint32_t id( char const* Str )
{
	if( Str != nullptr ) {
		return operator""_id( Str, strlen( Str ) );
	} else {
		return 0;
	}
}

constexpr inline uint32_t id( std::string_view Str )
{
	if( Str.data() != nullptr ) {
		return operator""_id( Str.data(), Str.length() );
	} else {
		return 0;
	}
}
