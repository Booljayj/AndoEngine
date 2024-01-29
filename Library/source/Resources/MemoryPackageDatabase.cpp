#include "Resources/MemoryPackageDatabase.h"

void Resources::MemoryPackageDatabase::Save(stdext::not_null<std::shared_ptr<Package>> package) {
	FPackageFlags expected = package->flags.load();
	while (package->flags.compare_exchange_weak(expected, expected + EPackageFlags::Dirty)) {}
}
