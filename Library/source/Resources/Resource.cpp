#include "Resources/Resource.h"
#include "Engine/StandardTypes.h"
#include "Resources/Package.h"
using namespace Reflection;

DEFINE_REFLECT_STRUCT(Resources, Resource)
	.Description("An object which can be shared between several other objects")
	.Variables({
		{ &Resources::Resource::name, "name"sv, "The unique identifier for this resource"sv, FVariableFlags::None() },
	});

namespace Resources {
	std::shared_ptr<Package const> Resource::GetPackage() const {
		auto const package = thread.package.LockInclusive();
		return *package;
	}

	Identifier Resource::GetIdentifier() const {
		auto const package = thread.package.LockInclusive();
		return Identifier{ *package ? (*package)->GetName() : StringID::None, name };
	}

	void Resource::Rename(StringID newName) {
		auto const package = thread.package.LockInclusive();

		if (*package) {
			//If this resource is in a package, we need to allow the package to perform the rename.
			(*package)->RenameResource(*this, newName);
		
		} else {
			//If this resource is not in a package, we can trivially rename it. The mutex for the package also covers changes to the name.
			name = newName;
		}
	}
}
