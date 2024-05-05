#include "Resources/FileDatabase.h"
#include "yaml-cpp/yaml.h"

namespace Resources {
	bool FileDatabase::IsPackageSaved(StringID name) const {
		std::filesystem::path const path = GetPath(name);
		return std::filesystem::exists(path);
	}

	void FileDatabase::SavePackage(stdext::shared_ref<Package> package) {
		PackageSource_YAML const source{ *package };

		std::filesystem::path const path = GetPath(package->GetName());

		//@todo File output should be handled by a separate thread, rather than blocking the calling thread here.
		std::ofstream file{ path, std::ios_base::out | std::ios_base::trunc };
		if (file.is_open() && file.good()) file << source.root;
	}

	void FileDatabase::ReloadPackage(stdext::shared_ref<Package> package) {
		//@todo Implement this. Not sure what the best way is to handle this. We can replace most resource contents in-place,
		//      but we also need to restore deleted resources and delete added resources.
	}

	void FileDatabase::DeletePackage(StringID name) {
		std::filesystem::path const path = GetPath(name);
		std::filesystem::remove(path);
	}

	PackageSource FileDatabase::LoadPackageSource(StringID name) {
		std::filesystem::path const path = GetPath(name);

		if (!std::filesystem::exists(path)) throw FormatType<std::runtime_error>("File '{}' not found, unable to load package source", path.generic_string());

		std::ifstream file{ path, std::ios_base::in };
		return PackageSource_YAML{ file };
	}

	bool FileDatabase::CanCreatePackage(StringID name) {
		//The presence of a file for this name means the package already exists but is not loaded, so we shouldn't allow it to be created.
		return !IsPackageSaved(name);
	}

	std::filesystem::path FileDatabase::GetPath(StringID name) {
		std::string_view const view = name.ToStringView();
		return std::filesystem::current_path() / std::filesystem::path{ view }.replace_extension("yaml");
	}
}
