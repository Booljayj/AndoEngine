#include "Resources/ResourceTypes.h"

namespace Resources {
	DEFINE_LOG_CATEGORY(Resources, Info);

	Identifier const Identifier::Invalid{ ~static_cast<ValueType>(0) };
}

DEFINE_REFLECT_ALIAS(Resources, Identifier);
