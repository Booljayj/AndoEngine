#pragma once
#include <ostream>
#include <istream>
#include <sstream>

namespace Serialization {
	struct ISerializer {
		virtual ~ISerializer() {}

		//Serialize to and from binary data streams
		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) = 0;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes = UINT32_MAX ) = 0;

		//Serialize to and from human-readable text streams
		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) = 0;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) = 0;
	};
}
