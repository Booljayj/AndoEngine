#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct ArrayTypeInfo;
}

namespace Serialization {
	struct ArraySerializer : public ISerializer
	{
	private:
		Reflection::ArrayTypeInfo const* Type;

	public:
		ArraySerializer() = delete;
		ArraySerializer( Reflection::ArrayTypeInfo const* InType );

		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) const override;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream ) const override;

		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) const override;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) const override;

	private:
		bool CanReadNextElementHeader( std::istream& Stream, std::streampos const& EndPosition ) const;

		void WriteArrayCount( std::ostream& Stream, void const* Data ) const;
		uint32_t ReadArrayCount( std::istream& Stream ) const;
	};
}