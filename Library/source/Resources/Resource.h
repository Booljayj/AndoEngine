#pragma once
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"
#include "Engine/Threads.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	struct Package;

	/** Initializer passed to the constructor when constructing a resource */
	struct Initializer {
		StringID name;
		size_t index;

		Initializer(StringID name, size_t index) : name(name), index(index) {}
	};

	/** Base class for an object that can be shared between many entities and scenes, and is tracked with reference counting */
	struct Resource : public std::enable_shared_from_this<Resource> {
		REFLECT_STRUCT(Resource, void);
		
		Resource(Initializer const& init) : name(init.name) {}
		virtual ~Resource() = default;

		inline StringID GetName() const { return name; }
		std::shared_ptr<Package const> GetPackage() const;
		Identifier GetIdentifier() const;

		/** Rename this resource. Will throw if the package for this resource already contains a resource with the new name. */
		void Rename(StringID newName);

	private:
		friend struct Package;
		friend struct ScopedPackageModifier;

		/** The unique name for this resource */
		StringID name;
		/** The flags that currently apply to this resource */
		FResourceFlags flags;

		struct {
			/** The package that this resource is currently contained in */
			ThreadSafe<std::shared_ptr<Package>> package;
		} thread;
	};

	/** A type that derives from Resource. Note this is not used for some type parameter constraints to allow them to be be forward-declared. */
	template<typename T>
	concept ResourceType = std::derived_from<T, Resource>;

	/** A handle that may point to a Resource object */
	template<typename T>
	using Handle = std::shared_ptr<T>;

	/** A handle that must point to a valid Resource object */
	template<typename T>
	using Reference = stdext::shared_ref<T>;
}

REFLECT(Resources::Resource, Struct);

inline bool operator==(Resources::Resource const& resource, StringID name) { return resource.GetName() == name; }
inline bool operator==(std::shared_ptr<Resources::Resource> const& resource, StringID name) { return resource && resource->GetName() == name; }
