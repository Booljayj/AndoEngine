#include "Resources/MemoryDatabase.h"

void Resources::MemoryDatabase::Save(stdext::not_null<std::shared_ptr<Package>> package) {
	package->flags -= EPackageFlags::Dirty;
}
