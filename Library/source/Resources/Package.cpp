#include "Resources/Package.h"
#include "Engine/StandardTypes.h"
#include "Engine/Temporary.h"
#include "Resources/Database.h"
#include "Resources/Resource.h"

namespace Resources {
	std::shared_ptr<Resource> Package::Find(StringID id) const {
		auto const contents = ts_contents.LockInclusive();
		auto const iter = contents->find(id);
		if (iter != contents->end()) return iter->second;
		else return nullptr;
	}
}
