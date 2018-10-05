#pragma once
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <ostream>

struct HeapBuffer
{
private:
	char* Begin;
	char* End;
	char* Current;

	size_t PeakUsage;
	size_t PeakOverflow;

public:
	HeapBuffer( size_t Capacity );
	~HeapBuffer();

	inline void Reset() noexcept { Current = Begin; }

	inline char* GetCursor() const noexcept { return Current; }
	inline void SetCursor( char* NewCurrent ) noexcept {
		Current = std::min( std::max( NewCurrent, Begin ), End ); //Clamp the input to ensure it's inside the buffer
		PeakUsage = std::max( PeakUsage, GetUsed() );
	}

	inline size_t GetCapacity() const noexcept { return End - Begin; }
	inline size_t GetAvailable() const noexcept { return End - Current; }
	inline size_t GetUsed() const noexcept { return Current - Begin; }
	inline size_t GetPeakUsage() const noexcept { return PeakUsage; }
	inline size_t GetPeakOverflow() const noexcept { return PeakOverflow; }

	inline bool Contains( void* Pointer ) const noexcept {
		return true &&
			static_cast<char*>( Pointer ) >= Begin &&
			static_cast<char*>( Pointer ) < End;
	}

	/** Returns an aligned array of elements inside this buffer, or nullptr if the buffer does not have enough space */
	template<typename T>
	inline T* Request( size_t Count = 1 ) noexcept {
		static_assert( sizeof(T) > 0, "Template type has zero size" );

		if( size_t const RequestedBytes = sizeof(T) * Count ) {
			size_t AvailableBytes = GetAvailable();
			void* AlignedCurrent = GetCursor();
			if( std::align( alignof(T), RequestedBytes, AlignedCurrent, AvailableBytes ) ) {
				SetCursor( static_cast<char*>( AlignedCurrent ) + RequestedBytes );
				return static_cast<T*>( AlignedCurrent );
			} else {
				//The actual overflow is probably a bit higher due to alignment, but this number does not need to be exact.
				PeakOverflow = std::max( PeakOverflow, RequestedBytes );
			}
		}
		return nullptr;
	}
};

/** std allocator that uses a buffer to manage allocations. */
template< typename T >
class TLinearAllocator
{
	template< typename U >
	friend class TLinearAllocator;

protected:
	HeapBuffer* Buffer = nullptr;

public:
	using value_type = T;
	using propagate_on_container_move_assignment = std::true_type;

	TLinearAllocator() = delete;
	TLinearAllocator( HeapBuffer& InBuffer )
	: Buffer( &InBuffer )
	{}

	TLinearAllocator( TLinearAllocator const& ) = default;

	template<typename U>
	TLinearAllocator( TLinearAllocator<U> const& Other )
	: Buffer( Other.Buffer )
	{}

	~TLinearAllocator() = default;

	T* allocate( size_t Count, void const* Hint = nullptr )
	{
		if( T* BufferRequest = Buffer->Request<T>( Count ) ) {
			return BufferRequest;
		} else {
			//default to heap allocation, we have exceeded the capacity of the temp allocator
			return static_cast<T*>( ::operator new( sizeof(T) * Count ) );
		}
	}

	void deallocate( T* Pointer, size_t Count )
	{
		if( !Buffer->Contains( Pointer ) ) {
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
	size_type max_size() const { return Buffer->GetCapacity(); }

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
