#pragma once
#include <vector>
#include "Serialization/Serializer.h"

namespace Reflection {
	struct StructTypeInfo;
	struct MemberVariableInfo;
}

namespace Serialization {
	struct StructSerializer : public ISerializer
	{
	private:
		Reflection::StructTypeInfo const* Type;
		std::vector<Reflection::MemberVariableInfo const*> CachedMemberVariables;

	public:
		StructSerializer() = delete;
		StructSerializer( Reflection::StructTypeInfo const* InType );

		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) override;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream ) override;

		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) override;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) override;

	private:
		void CacheVariables();

		void WriteVariableIdentifier( Reflection::MemberVariableInfo const* VariableInfo, std::ostream& Stream ) const;
		void WriteVariableStream( std::istream& VariableStream, std::ostream& Stream ) const;

		bool CanReadNextVariableHeader( std::istream& Stream, std::streampos const& EndPosition ) const;
		Reflection::MemberVariableInfo const* ReadVariableIdentifier( std::istream& Stream ) const;
	};
}
