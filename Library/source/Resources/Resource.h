#pragma once
#include "Engine/ManagedObject.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	/** Initializer passed to the constructor when constructing a resource */
	struct Initializer {
		Identifier id;
		size_t index;
	};

	/** Base class for an object that can be shared between many entities and scenes, and is tracked by resource counting */
	struct Resource : public ManagedObject {
		REFLECT_STRUCT(Resource, void);

		/** The unique identifier for this resource */
		Identifier id;
		/** The flags that currently apply to this resource */
		FResourceFlags flags;
		/** The human-readable name of this resource */
		std::string name;

		Resource(Initializer const& init) : id(init.id) {}
		virtual ~Resource() = default;
	};

	template<typename ResourceType>
	using Handle = ManagedObject::Handle<ResourceType>;
}

REFLECT(Resources::Resource, Struct);
