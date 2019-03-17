#include <fstream>
#include "EntityFramework/EntityManifest.h"

/**
 * Manifest files are text files with one manifest entry per line in the following format:
 * <EntityID> <FilePath>:<FileOffset>
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

void EntityManifest::LoadManifestFile( CTX_ARG, std::string_view Filename ) {
	// std::ifstream File{ Filename.data() };
	// const size_t BufferSize = CTX.Temp.GetAvailable();

	// std::streampos PrevPosition = File.tellg();
	// while( File.getline( CTX.Temp.GetCursor(), BufferSize ) ) {
	// 	const std::streampos NextPosition = File.tellg();
	// 	if( NextPosition < PrevPosition ) {
	// 		continue; //Bad read, next position should always be greater
	// 	}

	// 	const std::string_view ManifestLine{ CTX.Temp.GetCursor(), NextPosition - PrevPosition };
	// 	const size_t SplitterPosition = ManifestLine.find( " " );
	// 	const std::string_view EntityID = ManifestLine.substr( 0, SplitterPosition );
	// 	const std::string_view Filename = ManifestLine.substr( SplitterPosition, ManifestLine.size() - SplitterPosition );

	// 	PrevPosition = NextPosition;
	// }
}

EntityManifest::FileEntry const* EntityManifest::Find( const EntityID& ID ) const {
	// const auto Iter = Entries.find( ID );
	// if( Iter != Entries.end() ) {
	// 	return &(Iter->second);
	// }
	return nullptr;
}
