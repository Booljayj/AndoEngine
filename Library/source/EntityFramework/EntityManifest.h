#pragma once
#include "Engine/Context.h"
#include "Engine/STL.h"
#include "EntityFramework/EntityTypes.h"

/** Describes a set of entities that can be loaded from files */
struct EntityManifest {
	struct FileEntry {
		std::string filename;
		size_t offset = 0;
	};

	std::unordered_map<EntityAssetID, FileEntry> entries;

	/** Load entries from a file. Matching existing entries will be overwritten. */
	void LoadManifestFile(CTX_ARG, std::string_view filename);
	/** Find an entry for the provided id in this manifest */
	FileEntry const* Find(const EntityAssetID& id) const;
};
