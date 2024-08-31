#include "Resources/FileDatabase.h"
#include "yaml-cpp/yaml.h"

namespace Resources {
	void FileDatabase::ReloadPackage(StringID name) {
		//@todo Implement this. Not sure what the best way is to handle this. We can replace most resource contents in-place,
		//      but we also need to restore deleted resources and delete added resources.
	}

	void FileDatabase::DeletePackage(StringID name) {
		//@todo If package saving is performed on another thread, we'll need to sync this with the thread.
		//      Deleting a package that is pending a save should also cancel the save, if nothing else.
		std::filesystem::path const path = GetPath(name);
		std::filesystem::remove(path);
	}

	bool FileDatabase::IsPackageSaved(StringID name) const {
		std::filesystem::path const path = GetPath(name);
		return std::filesystem::exists(path);
	}

	bool FileDatabase::CanCreatePackage(StringID name) const {
		//The presence of a file for this name means the package already exists but is not loaded, so we shouldn't allow it to be created.
		return !IsPackageSaved(name);
	}

	bool FileDatabase::SavePackage(Package const& package) {
		PackageOutput_YAML const source{ package };

		std::filesystem::path const path = GetPath(package.GetName());

		//@todo File output should be handled by a separate thread, rather than blocking the calling thread here.
		std::ofstream file{ path, std::ios_base::out | std::ios_base::trunc };
		if (file.is_open() && file.good()) file << source.root;

		return true;
	}

	PackageInput FileDatabase::LoadPackageSource(StringID name) {
		std::filesystem::path const path = GetPath(name);

		if (!std::filesystem::exists(path)) throw FormatType<std::runtime_error>("File '{}' not found, unable to load package source", path.generic_string());

		std::ifstream file{ path, std::ios_base::in };
		return PackageInput_YAML{ file };
	}

	std::filesystem::path FileDatabase::GetPath(StringID name) {
		std::string_view const view = name.ToStringView();
		return std::filesystem::current_path() / "content"sv / std::filesystem::path{view}.replace_extension("yaml");
	}
}
