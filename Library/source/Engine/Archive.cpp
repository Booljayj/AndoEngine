#include "Engine/Archive.h"

void Archive::WriteBytes(Output& archive, std::span<std::byte const> source) {
	archive.buffer.append_range(source);
}

/** Read the specified number of bytes from the archive, and return a span of them. Throws if the archive does not contain the requested number of bytes. */
std::span<std::byte const> Archive::ReadBytes(Input& archive, size_t num) {
	//Create a subspan for the resulting bytes
	auto const result = archive.buffer.subspan(0, num);
	//Skip ahead in the archive to after the returned bytes
	archive.buffer = archive.buffer.subspan(num);
	return result;
}

/** Skip past the specified number of bytes without reading them. Throws if the archive does not contain the requested number of bytes. */
void Archive::Skip(Input& archive, size_t num) {
	archive.buffer = archive.buffer.subspan(num);
}

/** Create a new archive that will read a subset of this archive, containing the provided number of bytes. Does not modify the source archive. Throws if the archive does not contain the requested number of bytes */
Archive::Input Archive::Subset(Input const& archive, size_t num) {
	return Input{ archive.buffer.subspan(0, num) };
}
