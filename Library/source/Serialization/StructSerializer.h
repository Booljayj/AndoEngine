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

		virtual void Load() override;

		virtual void SerializeBinary( void const* Data, std::ostream& Stream ) const override;
		virtual bool DeserializeBinary( void* Data, std::istream& Stream, uint32_t NumBytes ) const override;

		virtual void SerializeText( void const* Data, std::ostringstream& Stream ) const override;
		virtual bool DeserializeText( void* Data, std::istringstream& Stream ) const override;
	};
}
