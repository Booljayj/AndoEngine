#include <unordered_map>
#include <string>
#include <string_view>
#include "Engine/Context.h"
#include "EntityFramework/Entity.h"

/** Describes a set of entities that can be loaded from files */
struct EntityManifest {
	struct FileEntry {
		std::string filename;
		size_t offset = 0;
	};

	std::unordered_map<EntityID, FileEntry> entries;

	/** Load entries from a file. Matching existing entries will be overwritten. */
	void LoadManifestFile(CTX_ARG, std::string_view filename);
	/** Find an entry for the provided id in this manifest */
	FileEntry const* Find(const EntityID& id) const;
};
