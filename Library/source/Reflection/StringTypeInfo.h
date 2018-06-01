#pragma once
#include "Reflection/TypeInfo.h"

namespace Reflection {
	struct StringTypeInfo : public TypeInfo {
		StringTypeInfo();
		virtual int8_t Compare( void const* A, void const* B ) const override;
	};
}
