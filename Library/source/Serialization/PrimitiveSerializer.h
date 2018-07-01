#pragma once
#include "Serialization/Serializer.h"
#include "Serialization/ByteUtility.h"

namespace Serialization {
	template<typename T>
	struct TPrimitiveSerializer : public ISerializer
	{
		static_assert( sizeof( T ) < UINT32_MAX, "Serializing primitive types that are larger than UINT32_MAX is not supported" );

		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) override {
			uint32_t const SIZE = sizeof( T );
			WriteLE( &SIZE, Stream );
			WriteLE( reinterpret_cast<T const*>( Data ), Stream );
		}

		virtual bool DeserializeBinary( void* Data, std::istream& Stream ) override {
			uint32_t NumBytes = 0;
			ReadLE( &NumBytes, Stream );
			if( NumBytes == sizeof( T ) ) {
				ReadLE( reinterpret_cast<T*>( Data ), Stream );
				return true;
			} else {
				Stream.seekg( Stream.tellg() + std::streamoff{ NumBytes } );
				return false;
			}
		}

		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) override {
			Stream << *static_cast<T const*>( Data );
		}
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) override {
			Stream >> *static_cast<T*>( Data );
			return true;
		}
	};
}
