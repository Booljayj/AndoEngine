#pragma once
#include "Engine/StandardTypes.h"
#include "Resources/Package.h"

namespace Reflection { struct StructTypeInfo; }

namespace Resources {
	struct Resource;

	/** Returns whether a reference to the package can be saved */
	bool CanSavePackage(StringID const& name);

	/** Gather the packages on which the provided package depends */
	std::unordered_set<StringID> GatherPackageDependencies(Package const& package);
	/** Gather the packages on which the provided resources depend */
	std::unordered_set<StringID> GatherPackageDependencies(Package::ContentsContainerType const& contents);
	/** Gather the packages on which the provided instance depends */
	void GatherPackageDependencies(Reflection::StructTypeInfo const& type, void const* instance, std::unordered_set<StringID>& dependencies);
}
