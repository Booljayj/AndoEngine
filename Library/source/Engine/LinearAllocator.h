#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <ostream>

struct HeapBuffer
{
private:
	uint8_t* Data;
	size_t Capacity;
	size_t Used;
	size_t Peak;

public:
	HeapBuffer( size_t Capacity );
	~HeapBuffer();

	inline void* GetData( size_t Offset = 0 ) const { return Data + Offset; }
	inline size_t GetCapacity() const { return Capacity; }
	inline size_t GetUsed() const { return Used; }
	inline size_t GetPeak() const { return Peak; }

	inline bool IsValid() const { return Used <= Capacity; }
	inline bool Contains( void* Pointer ) const {
		return true &&
			static_cast<uint8_t*>( Pointer ) >= Data &&
			static_cast<uint8_t*>( Pointer ) < ( Data + Capacity );
	}

	inline void SetUsed( size_t NewUsed )
	{
		Peak = std::max( NewUsed, Peak );
		Used = NewUsed;
	}

	inline void Reset() { Used = 0; }

	/** Returns an aligned array of elements inside this buffer, or nullptr if the buffer does not have enough space */
	template<typename T>
	T* Request( size_t Count = 1 ) noexcept {
		static_assert( sizeof(T) > 0, "Template type has zero size" );
		constexpr size_t ElementSize = sizeof( T );
		constexpr size_t ElementAlignment = alignof( T );

		const size_t CurrentUsed = GetUsed();
		//Find the difference between the current used point and the next aligned point
		const size_t AlignedCurrentUsed = CurrentUsed + ( ElementAlignment - ( CurrentUsed % ElementAlignment ) ) % ElementAlignment;
		const size_t NewUsed = AlignedCurrentUsed + ( ElementSize * Count );

		if( NewUsed <= Capacity ) {
			SetUsed( NewUsed );
			return static_cast<T*>( GetData( AlignedCurrentUsed ) );
		} else {
			return nullptr;
		}
	}
};

/** std allocator that uses a buffer to manage allocations. */
template< typename T >
class TLinearAllocator
{
	template< typename U >
	friend class TLinearAllocator;

protected:
	HeapBuffer* Allocator = nullptr;

	static constexpr size_t ElementSize = sizeof( T );
	static constexpr size_t ElementAlignment = alignof( T );

public:
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	TLinearAllocator() = delete;
	TLinearAllocator( HeapBuffer& InAllocator )
	: Allocator( &InAllocator )
	{}

	TLinearAllocator( TLinearAllocator const& ) = default;

	template<typename U>
	TLinearAllocator( TLinearAllocator<U> const& Other )
	: Allocator( Other.Allocator )
	{}

	~TLinearAllocator() = default;

	T* allocate( size_t Count, void const* Hint = nullptr )
	{
		if( T* BufferRequest = Allocator->Request<T>( Count ) ) {
			return BufferRequest;
		} else {
			//default to heap allocation, we have exceeded the capacity of the temp allocator
			return static_cast<T*>( ::operator new( ElementSize * Count ) );
		}
	}

	void deallocate( T* Pointer, size_t Count )
	{
		if( !Allocator->Contains( Pointer ) ) {
			::operator delete( Pointer );
		}
	}


	//-----------------------------------
	//Boilerplate for older C++ libraries
	using pointer = T*;
	using const_pointer = T const*;
	using reference = T&;
	using const_reference = T const&;
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
