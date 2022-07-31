#pragma once
#include "Engine/LogCommands.h"
#include "Engine/ManagedObject.h"
#include "Engine/Reflection.h"
#include "Engine/STL.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	/** Base class for an object that can be shared between many entities and scenes, and is tracked by resource counting */
	struct Resource : public ManagedObject {
		DECLARE_STRUCT_REFLECTION_MEMBERS(Resource, void);

		/** The unique identifier for this resource */
		Identifier id;
		/** The flags that currently apply to this resource */
		FResourceFlags flags;
		/** The human-readable name of this resource */
		std::string name;

		virtual ~Resource() = default;
	};

	template<typename ResourceType>
	using Handle = ManagedObject::Handle<ResourceType>;
}

DECLARE_REFLECT(Resources::Resource, Struct);