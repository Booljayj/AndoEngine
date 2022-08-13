#pragma once
#include "Engine/StandardTypes.h"

//These classes are allocator adapters, not allocators. They provide an easy way to pass in an allocator type
//to a template whithout needing the allocated data type, which can be internal to the template. They do this
//by exposing a "Type" member which is a template that always takes the allocated data type as a parameter.

struct DefaultAllocatorFactory {
	template<typename DataType>
	using TAllocator = std::allocator<DataType>;
};

struct StackAllocatorFactory {
	template<typename DataType>
	using TAllocator = TLinearAllocator<DataType>;
};

/* Not implemented yet
template<size_t FixedCount>
struct FixedAllocatorFactory {
	typename<typename DataType>
	using TAllocator = FixedAllocatorImpl<DataType, FixedCount>;
};

template<size_t InlineCount, typename FallbackAllocator>
struct InlineAllocatorFactory {
	template<typename DataType>
	using TAllocator = InlineAllocatorImpl<DataType, InlineAllocator, FallbackAllocator::Type<DataType>>;
};
*/
