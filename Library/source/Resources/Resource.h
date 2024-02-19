#pragma once
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Engine/Threads.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	struct Cache;
	struct Package;

	/** Base class for an object that can be shared between many entities and scenes, and is tracked with reference counting */
	struct Resource : public std::enable_shared_from_this<Resource> {
		REFLECT_STRUCT(Resource, void);
		
		Resource(StringID name) : ts_description(name) {}
		virtual ~Resource() = default;

		StringID GetName() const;
		std::shared_ptr<Package const> GetPackage() const;
		Identifier GetIdentifier() const;

	private:
		friend struct Database;
		friend struct Package;
		friend struct ResourceUtility;

		/** The fundamental information that describes a resource */
		struct ResourceDescription {
			/** The unique name for this resource */
			StringID name;
			/** The flags that currently apply to this resource */
			FResourceFlags flags;
			/** The package in which this resource is contained */
			std::weak_ptr<Package> package;

			ResourceDescription(StringID name) : name(name) {}
		};

		/** The fundamental information that describes this resource, which must be thread-safe. */
		ThreadSafe<ResourceDescription> ts_description;
	};

	namespace Concepts {
		/** A type that derives from Resource. Note this is not used for some type parameter constraints to allow them to be be forward-declared. */
		template<typename T>
		concept DerivedFromResource =
			std::derived_from<T, Resource> and
			Reflection::Concepts::ReflectedStruct<T>;
	}

	/** A handle that may point to a Resource object */
	template<typename T>
	using Handle = std::shared_ptr<T>;

	/** A reference that must point to a valid Resource object */
	template<typename T>
	using Reference = stdext::shared_ref<T>;
}

REFLECT(Resources::Resource, Struct);

inline bool operator==(Resources::Resource const& resource, StringID name) { return resource.GetName() == name; }
inline bool operator==(std::shared_ptr<Resources::Resource> const& resource, StringID name) { return resource && resource->GetName() == name; }
