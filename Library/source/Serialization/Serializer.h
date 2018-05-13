#pragma once
#include <ostream>
#include <istream>
#include <sstream>

namespace Serialization {
	struct ISerializer {
		virtual ~ISerializer() {}

		//Prepare this serializer to begin serializing data, caching any appropriate data
		virtual void Load() {}

		//Serialize to and from binary data streams
		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) const = 0;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes = UINT32_MAX ) const = 0;

		//Serialize to and from human-readable text streams
		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) const = 0;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) const = 0;
	};
}
