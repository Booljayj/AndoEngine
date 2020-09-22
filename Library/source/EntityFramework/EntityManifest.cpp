#include "Engine/STL.h"
#include "EntityFramework/EntityManifest.h"

/**
 * Manifest files are text files with one manifest entry per line in the following format:
 * <entityID> <FilePath>:<FileOffset>
 *
 * For example, a manifest file may contain the following
 * 342F451B Data/Game/Character.edf:0
 * 547CB3F2 Data/Game/Character.edf:1093
 * 1394164A Data/Environment/Chair.edf:74836
 * 75E12AB1 Data/6E4A2F12.edf:984
 *
 * Manifest files are text, but are meant to be tool-generated, so the formatting requirements are very strict.
 * They are kept as text to make inspection much easier, especially with respect to overrides.
 */

void EntityManifest::LoadManifestFile(CTX_ARG, std::string_view filename) {
	// std::ifstream file{ filename.data() };
	// size_t const bufferSize = CTX.temp.GetAvailable();

	// std::streampos prevPosition = file.tellg();
	// while (file.getline(CTX.temp.GetCursor(), bufferSize)) {
	// 	std::streampos const nextPosition = file.tellg();
	// 	if (nextPosition < prevPosition) {
	// 		continue; //Bad read, next position should always be greater
	// 	}

	// 	std::string_view const manifestLine{CTX.temp.GetCursor(), nextPosition - prevPosition};
	// 	size_t const splitterPosition = manifestLine.find( " " );
	// 	std::string_view const id = manifestLine.substr( 0, splitterPosition );
	// 	std::string_view const filename = manifestLine.substr( splitterPosition, manifestLine.size() - splitterPosition );

	// 	prevPosition = nextPosition;
	// }
}

EntityManifest::FileEntry const* EntityManifest::Find(const EntityAssetID& id) const {
	// const auto iter = entries.find(id);
	// if (iter != entries.end()) {
	// 	return &(iter->second);
	// }
	return nullptr;
}
