#pragma once
#include "Engine/Archive.h"
#include "Engine/Reflection.h"
#include "Engine/Core.h"
#include "Engine/SmartPointers.h"
#include "Engine/StringID.h"
#include "Engine/Threads.h"
#include "Resources/ResourceTypes.h"
#include "ThirdParty/yaml.h"

namespace Resources {
	struct Cache;
	struct Package;

	/** Base class for an object that can be shared between many entities and scenes, and is tracked with reference counting */
	struct Resource : public std::enable_shared_from_this<Resource> {
		DECLARE_STRUCT_REFLECTION_MEMBERS(Resource, void);
		using ExternalObjectBaseType = Resource;
		
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

	/** Generic interface for classes that can provide resources based on an identifier */
	struct IResourceProvider {
		/** Creates a scope within which serialization can retrieve resource handles using a serialized identifier. These scopes must not be nested. */
		struct ProviderScope {
			ProviderScope(IResourceProvider const& provider) {
				assert(scoped_provider == nullptr);
				scoped_provider = &provider;
			}
			~ProviderScope() {
				scoped_provider = nullptr;
			}
		};

		/** Creates a new provider scope and returns it */
		[[nodiscard]] ProviderScope CreateResourceProviderScope() const { return ProviderScope(*this); }
		/** Get the resource provider for the current scope. */
		static IResourceProvider const* GetResourceProvider() { return scoped_provider; }

		virtual std::shared_ptr<Resource> FindResource(Identifier id) const noexcept = 0;

	private:
		static IResourceProvider const* scoped_provider;
	};

	namespace Concepts {
		/** A type that derives from Resource. Note this is not used for some type parameter constraints to allow them to be be forward-declared. */
		template<typename T>
		concept DerivedFromResource =
			std::derived_from<T, Resource> and
			Reflection::Concepts::ReflectedStructType<T>;
	}

	/** A handle that may point to a Resource object */
	template<typename T>
	using Handle = std::shared_ptr<T>;
}

REFLECT(Resources::Resource, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(Resources::Resource);
DEFINE_DEFAULT_YAML_SERIALIZATION(Resources::Resource);

inline bool operator==(Resources::Resource const& resource, StringID name) { return resource.GetName() == name; }
inline bool operator==(std::shared_ptr<Resources::Resource> const& resource, StringID name) { return resource && resource->GetName() == name; }

namespace Archive {
	struct ResourcePointerSerializer {
	protected:
		static void WriteHandle(Output& archive, std::shared_ptr<Resources::Resource> const& handle);
		static std::shared_ptr<Resources::Resource> ReadHandle(Input& archive);
	};

	template<std::derived_from<Resources::Resource> ResourceType>
	struct Serializer<std::shared_ptr<ResourceType>> : public ResourcePointerSerializer {
		static void Write(Output& archive, std::shared_ptr<ResourceType> const& handle) {
			WriteHandle(archive, handle);
		}
		static void Read(Input& archive, std::shared_ptr<ResourceType>& handle) {
			using namespace Resources;
			if (std::shared_ptr<Resources::Resource> const resource = ReadHandle(archive)) {
				if (resource->GetTypeInfo().IsChildOf<ResourceType>()) {
					handle = std::static_pointer_cast<ResourceType>(resource);
				} else {
					LOG(Resources, Warning, "A resource was loaded for identifier %s, but it does not match the handle type. The handle will be empty.", resource->GetIdentifier());
					handle = nullptr;
				}
			} else {
				handle = nullptr;
			}
		}
	};
}

namespace YAML {
	struct ResourcePointerConverter {
	protected:
		static Node EncodeHandle(std::shared_ptr<Resources::Resource> const& handle);
		static std::shared_ptr<Resources::Resource> DecodeHandle(Node const& node);
	};

	template<std::derived_from<Resources::Resource> ResourceType>
	struct convert<std::shared_ptr<ResourceType>> : public ResourcePointerConverter {
		static Node encode(std::shared_ptr<ResourceType> const& handle) {
			return EncodeHandle(handle);
		}
		static bool decode(Node const& node, std::shared_ptr<ResourceType>& handle) {
			using namespace Resources;
			if (std::shared_ptr<Resources::Resource> const resource = DecodeHandle(node)) {
				if (resource->GetTypeInfo().IsChildOf<ResourceType>()) {
					handle = std::static_pointer_cast<ResourceType>(resource);
				}
				else {
					LOG(Resources, Warning, "A resource was loaded for identifier %s, but it does not match the handle type. The handle will be empty.", resource->GetIdentifier());
					handle = nullptr;
				}
			}
			else {
				handle = nullptr;
			}
			return true;
		}
	};
}
