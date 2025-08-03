#pragma once
#include "Engine/Core.h"
#include "Engine/SmartPointers.h"
#include "Engine/StringID.h"

namespace Resources {
	struct Package;
	struct Resource;

	/** Contains utility methods for manipulating resources and packages */
	struct ResourceUtility {
		/** Move the resource from its current package to the new destination package. This is an atomic operation that will throw if it can't succeed, resources must belong to a package. */
		static void MoveResource(std::shared_ptr<Resource> resource, std::shared_ptr<Package> const& destination);
	
		/** Rename the package. Will throw if the new name is not unique within the database that contains the package. */
		static void RenamePackage(std::shared_ptr<Package> package, StringID name);

		/** Rename the resource. Will throw if the new name is not unique within the package that contains the resource. */
		static void RenameResource(std::shared_ptr<Resource> resource, StringID name);
	};
}
