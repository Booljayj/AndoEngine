#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct StructTypeInfo;
	struct VariableInfo;
}

namespace Serialization {
	struct StructSerializer : public ISerializer {
	private:
		Reflection::StructTypeInfo const* Type;

	public:
		StructSerializer() = delete;
		StructSerializer( Reflection::StructTypeInfo const* InType );

		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) const override;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream ) const override;

	private:
		void WriteVariableIdentifier( Reflection::VariableInfo const* VariableInfo, std::ostream& Stream ) const;
		void WriteVariableStream( std::istream& VariableStream, std::ostream& Stream ) const;

		bool CanReadNextVariableHeader( std::istream& Stream, std::streampos const& EndPosition ) const;
		Reflection::VariableInfo const* ReadVariableIdentifier( std::istream& Stream ) const;
		/** Handle a data block for an unknown variable */
		void HandleUnknownDataBlock( std::istream& Stream ) const;
	};
}
