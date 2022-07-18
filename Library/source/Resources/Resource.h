#pragma once
#include "Engine/LogCommands.h"
#include "Engine/ManagedObject.h"
#include "Engine/STL.h"
#include "Reflection/StructTypeInfo.h"
#include "Reflection/TypeResolver.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	/** Base class for an object that can be shared between many entities and scenes, and is tracked by resource counting */
	struct Resource : public ManagedObject {
		/** The unique identifier for this resource */
		Identifier id;
		/** The flags that currently apply to this resource */
		FResourceFlags flags;
		/** The human-readable name of this resource */
		std::string name;

		DECLARE_REFLECTION_MEMBERS(Resource, void);
	};

	template<typename ResourceType>
	using Handle = ManagedObject::Handle<ResourceType>;
}

REFLECT(Resources::Resource);
