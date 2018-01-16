#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <ostream>

class LinearAllocatorData
{
	uint8_t* Data;
	size_t Capacity;
	size_t Used;
	size_t Peak; //@todo: use a define to get rid of this at runtime

public:
	LinearAllocatorData( size_t RealCapacity );
	~LinearAllocatorData();

	inline uint8_t* GetData( const size_t Offset = 0 ) const { return Data + Offset; }
	inline size_t GetCapacity() const { return Capacity; }
	inline size_t GetUsed() const { return Used; }
	inline size_t GetPeak() const { return Peak; }

	inline bool IsValid() const { return Used <= Capacity; }
	inline bool Contains( uint8_t* Pointer ) const { return ( Pointer >= Data ) && ( Pointer < ( Data + Capacity ) ); }

	inline void SetUsed( size_t NewUsed )
	{
		Peak = std::max( NewUsed, Peak );
		Used = NewUsed;
	}

	inline void Reset() { Used = 0; }

	friend std::ostream& operator<<( std::ostream& Stream, LinearAllocatorData& Alloc );
};

/** std allocator that uses a linear allocator data struct to manage allocations. */
template< typename T >
class TLinearAllocator
{
	template< typename U >
	friend class TLinearAllocator;
protected:
	LinearAllocatorData* Allocator = nullptr;

	static constexpr size_t ElementSize = sizeof( T );
	static constexpr size_t ElementAlignment = alignof( T );

public:
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	TLinearAllocator() = delete;
	TLinearAllocator( LinearAllocatorData& InAllocator )
	: Allocator( &InAllocator )
	{}

	TLinearAllocator( const TLinearAllocator& ) = default;

	template<typename U>
	TLinearAllocator( const TLinearAllocator<U>& Other )
	: Allocator( Other.Allocator )
	{}

	~TLinearAllocator() = default;

	T* allocate( size_t Count, const void* Hint = nullptr )
	{
		const size_t PrevUsed = Allocator->GetUsed();
		const size_t AlignmentCorrection = ( ElementAlignment - ( PrevUsed % ElementAlignment ) ) % ElementAlignment;
		const size_t NewUsed = PrevUsed + AlignmentCorrection + ( ElementSize * Count );

		Allocator->SetUsed( NewUsed );
		if( !Allocator->IsValid() )
		{
			//default to heap allocation, we have exceeded the capacity of the temp allocator
			return reinterpret_cast<T*>( ::operator new( ElementSize * Count ) );
		}
		else
		{
			return reinterpret_cast<T*>( Allocator->GetData( PrevUsed ) );
		}
	}

	void deallocate( T* Pointer, size_t Count )
	{
		if( !Allocator->Contains( reinterpret_cast<uint8_t*>( Pointer ) ) )
		{
			::operator delete( reinterpret_cast<void*>( Pointer ) );
		}
	}


	//-----------------------------------
	//Boilerplate for older C++ libraries
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	template< class U > struct rebind { typedef TLinearAllocator<U> other; };
	using is_always_equal = std::false_type;

	pointer address( reference x ) const noexcept { return &x; }
	const_pointer address( const_reference x ) const noexcept { return &x; }
	size_type max_size() const { return Allocator->GetCapacity(); }

	template< class U, class... Args >
	void construct( U* Pointer, Args&&... args )
	{
		::new( (void*)Pointer ) U( std::forward<Args>( args )... );
	}
	template< class U >
	void destroy( U* Pointer )
	{
		Pointer->~U();
	}
};
