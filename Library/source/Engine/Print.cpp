#include "Engine/Print.h"
#include "Engine/LinearAllocator.h"

void l_printf_internal( CTX_ARG, const char* Format, va_list args, char*& OutPtr, size_t& OutLength )
{
	LinearAllocatorData& Alloc = CTX.Temp;
	OutPtr = reinterpret_cast<char*>( Alloc.GetData( Alloc.GetUsed() ) );
	const size_t MaxLength = Alloc.GetCapacity() - Alloc.GetUsed();

	OutLength = vsnprintf( OutPtr, MaxLength, Format, args );
	Alloc.SetUsed( Alloc.GetUsed() + OutLength );
}

const char* l_printf( CTX_ARG, const char* Format, ... )
{
	va_list args;
	va_start( args, Format );

	char* DataPtr;
	size_t Length;

	l_printf_internal( CTX, Format, args, DataPtr, Length );

	va_end( args );
	return DataPtr;
}

l_string l_sprintf( CTX_ARG, const char* Format, ... )
{
	va_list args;
	va_start( args, Format );

	char* DataPtr;
	size_t Length;

	l_printf_internal( CTX, Format, args, DataPtr, Length );

	va_end( args );
	return l_string{ DataPtr, Length, CTX.Temp };
}
