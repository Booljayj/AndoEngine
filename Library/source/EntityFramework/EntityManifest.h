#include <unordered_map>
#include <string>
#include <string_view>
#include "Engine/Context.h"
#include "EntityFramework/Entity.h"

/** Describes a set of entities that can be loaded from files */
struct EntityManifest {
	struct FileEntry {
		std::string Filename;
		size_t Offset = 0;
	};

	std::unordered_map<EntityID, FileEntry> Entries;

	/** Load entries from a file. Matching existing entries will be overwritten. */
	void LoadManifestFile( CTX_ARG, std::string_view Filename );
	/** Find an entry for the provided ID in this manifest */
	FileEntry const* Find( const EntityID& ID ) const;
};
