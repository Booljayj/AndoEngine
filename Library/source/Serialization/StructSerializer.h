#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct StructTypeInfo;
	struct VariableInfo;
}

namespace Serialization {
	struct StructSerializer : public ISerializer
	{
	private:
		Reflection::StructTypeInfo const* Type;
		std::vector<Reflection::VariableInfo const*> CachedMemberVariables;

	public:
		StructSerializer() = delete;
		StructSerializer( Reflection::StructTypeInfo const* InType );

		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) const override;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream ) const override;

		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) const override;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) const override;

		void Initialize();

	private:
		void WriteVariableIdentifier( Reflection::VariableInfo const* VariableInfo, std::ostream& Stream ) const;
		void WriteVariableStream( std::istream& VariableStream, std::ostream& Stream ) const;

		bool CanReadNextVariableHeader( std::istream& Stream, std::streampos const& EndPosition ) const;
		Reflection::VariableInfo const* ReadVariableIdentifier( std::istream& Stream ) const;
		/** Handle a data block for an unknown variable */
		void HandleUnknownDataBlock( std::istream& Stream ) const;
	};
}
