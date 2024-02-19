#include "Resources/Resource.h"
#include "Engine/StandardTypes.h"
#include "Resources/Package.h"
using namespace Reflection;

DEFINE_REFLECT_STRUCT(Resources, Resource)
.Description("An object which can be shared between several other objects");

namespace Resources {
	StringID Resource::GetName() const {
		auto const description = ts_description.LockInclusive();
		return description->name;
	}

	std::shared_ptr<Package const> Resource::GetPackage() const {
		auto const description = ts_description.LockInclusive();
		return description->package.lock();
	}

	Identifier Resource::GetIdentifier() const {
		auto const description = ts_description.LockInclusive();
		auto const pinned = description->package.lock();
		return Identifier{ pinned ? pinned->GetName() : StringID::None, description->name };
	}
}
