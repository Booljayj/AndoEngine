#include "Serialization/Serializer.h"
#include "Serialization/ByteUtility.h"

namespace Serialization {
	template<typename T>
	struct TPrimitiveSerializer : public ISerializer
	{
		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) override {
			LittleEndianByteSerializer<sizeof(T)>::Write( static_cast<char const*>( Data ), Stream );
		}
		virtual bool DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes ) override {
			if( NumBytes == sizeof( T ) ) {
				LittleEndianByteSerializer<sizeof(T)>::Read( static_cast<char*>( Data ), Stream );
				return true;
			} else {
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
