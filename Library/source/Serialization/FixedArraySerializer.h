#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct FixedArrayTypeInfo;
}

namespace Serialization {
	struct FixedArraySerializer : public ISerializer
	{
	private:
		Reflection::FixedArrayTypeInfo const* Type;

	public:
		FixedArraySerializer() = delete;
		FixedArraySerializer( Reflection::FixedArrayTypeInfo const* InType );

		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) override;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes ) override;

		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) override;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) override;
	};
}