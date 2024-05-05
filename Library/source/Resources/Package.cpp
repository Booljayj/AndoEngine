#include "Resources/Package.h"
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"
#include "Resources/Database.h"
#include "Resources/Resource.h"

Resources::Package::Package(std::shared_ptr<Database> owner, StringID name, ContentsContainerType const& contents)
	: owner(owner), name(name), ts_contents(contents)
{
	auto const shared_this = shared_from_this();

	for (auto const [name, resource] : contents) {
		auto description = resource->ts_description.LockExclusive();

		if (auto const existing = description->package.lock()) {
			throw FormatType<std::runtime_error>("Cannot create package with contents, {} is already part of package {}", description->name, existing->name);
		}

		description->package = shared_this;
	}
}
