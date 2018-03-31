#pragma once
#include <cassert>

#define VIEW_CHECKS 1

template<typename T>
struct ArrayView
{
	static_assert( !std::is_void<T>::value, "Cannot create an array view for void type" );

private:
	T* Begin = nullptr;
	T* End = nullptr;

public:
	ArrayView() = default;
	ArrayView( T* InBegin, T* InEnd )
	: Begin( InBegin ), End( InEnd )
	{
#if VIEW_CHECKS
		assert( Begin <= End );
#endif
	}
	ArrayView( T* InBegin, size_t InSize )
	: ArrayView( InBegin, InBegin + InSize )
	{}

	explicit operator bool() const { return Begin != nullptr && Begin != End; }

	T& operator[]( size_t Index ) const { return Begin[Index]; }

	size_t size() { return End - Begin; }

	T* begin() { return Begin; }
	T* end() { return End; }

	T const* begin() const { return Begin; }
	T const* end() const { return End; }
};

template<typename T>
struct VectorView
{
	static_assert( !std::is_void<T>::value, "Cannot create a vector view for void type" );

private:
	T* Begin = nullptr;
	T* End = nullptr;
	T* Capacity = nullptr;

public:
	VectorView() = default;
	VectorView( T* InBegin, T* InEnd, T* InCapacity )
	: Begin( InBegin ), End( InEnd ), Capacity( InCapacity )
	{
#if VIEW_CHECKS
		assert( Begin <= End );
		assert( Begin <= Capacity );
		assert( End <= Capacity );
#endif
	}
	VectorView( T* InBegin, size_t InSize, size_t InCapacity )
	: VectorView( InBegin, InBegin + InSize, InBegin + InCapacity )
	{}
	VectorView( T* InBegin, T* InCapacity )
	: VectorView( InBegin, InBegin, InCapacity )
	{}
	VectorView( T* InBegin, size_t InCapacity )
	: VectorView( InBegin, InBegin, InBegin + InCapacity )
	{}

	explicit operator bool() const { return Begin != nullptr && Begin != End; }

	T& operator[]( size_t Index ) const { return Begin[Index]; }

	size_t size() { return End - Begin; }
	size_t capacity() { return Capacity - Begin; }

	void push( T const& N )
	{
#if VIEW_CHECKS
		assert( End != Capacity );
#endif
		*End = N;
		++End;
	}
	T& pop()
	{
#if VIEW_CHECKS
		assert( End != Begin );
#endif
		--End;
		return *End;
	}

	T* begin() { return Begin; }
	T* end() { return End; }

	T const* begin() const { return Begin; }
	T const* end() const { return End; }
};
