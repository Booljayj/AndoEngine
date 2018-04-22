#pragma once
#include <ostream>
#include <istream>
#include <sstream>

struct ISerializer {
	virtual ~ISerializer() {}

	virtual void SerializeBinary( void const* Data, std::ostream& Stream ) const = 0;
	virtual bool DeserializeBinary( std::istream const& Stream, void* Data ) const = 0;

	virtual void SerializeText( void const* Data, std::ostringstream& Stream ) const = 0;
	virtual bool DeserializeText( std::istringstream const& Stream, void* Data ) const = 0;
};
