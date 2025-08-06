#include "Resources/StreamingUtils.h"
#include "Engine/Reflection.h"
#include "Engine/StringID.h"
#include "Resources/Database.h"
#include "Resources/Package.h"
#include "Resources/Resource.h"
#include "Resources/ResourceTypes.h"

namespace Resources {
	bool CanSavePackage(StringID const& package) {
		return package != StringID::None && package != StringID::Temporary;
	}

	std::unordered_set<StringID> GatherPackageDependencies(Package const& package) {
		return GatherPackageDependencies(*package.GetContentsView());
	}
	std::unordered_set<StringID> GatherPackageDependencies(Package::ContentsContainerType const& contents) {
		std::unordered_set<StringID> dependencies;
		for (auto const& pair : contents) {
			Reflection::StructTypeInfo const& type = pair.second->GetTypeInfo();
			GatherPackageDependencies(type, pair.second.get(), dependencies);
		}
		return dependencies;
	}
	std::unordered_set<StringID> GatherPackageDependencies(Resource const& resource) {
		std::unordered_set<StringID> dependencies;
		Reflection::StructTypeInfo const& type = resource.GetTypeInfo();
		GatherPackageDependencies(type, &resource, dependencies);
		return dependencies;
	}

	void GatherPackageDependencies(Reflection::StructTypeInfo const& type, void const* instance, std::unordered_set<StringID>& dependencies) {
		using namespace Reflection;

		for (Reflection::StructTypeInfo const* current = &type; current; current = current->base) {
			for (std::unique_ptr<VariableInfo const> const& variable : current->GetVariables()) {
				//Skip variables that are explicitly not serialized, or which are deprecated (deprecated variables can be loaded, but will not be saved)
				if (variable->flags.HasAny(EVariableFlags::NonSerialized, EVariableFlags::Deprecated)) continue;

				//If this variable is a reference to a Resource object, record it as a dependency
				if (auto const* reference = Cast<ReferenceTypeInfo>(variable->type)) {
					if (reference->base->IsChildOf<Resource>()) {
						//This variable appears to be a reference to a Resource object. Retrieve the reference and add it as a dependency if it is saved.
						auto const dependency = std::static_pointer_cast<Resource const>(reference->GetImmutable(variable->GetImmutable(instance)));
						if (dependency) {
							Identifier const identifier = dependency->GetIdentifier();
							if (CanSavePackage(identifier.package)) dependencies.emplace(identifier.package);
						}
					}

					//If this variable is a struct, recurse into the struct to check its variables for dependencies
				}
				else if (auto const* struct_type = Cast<StructTypeInfo>(variable->type)) {
					GatherPackageDependencies(*struct_type, variable->GetImmutable(instance), dependencies);
				}
			}
		}
	}

}
