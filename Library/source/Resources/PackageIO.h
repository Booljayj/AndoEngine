#pragma once
#include "Engine/Archive.h"
#include "Resources/Package.h"
#include "Engine/Reflection.h"
#include "Engine/Core.h"
#include "Engine/Set.h"
#include "Engine/StringID.h"
#include "Engine/Variant.h"
#include "ThirdParty/yaml.h"

namespace Resources {
	struct PackageOutput_Binary {
		std::vector<std::byte> bytes;

		PackageOutput_Binary(Package const& package);
		void Write(std::ostream& stream) const;

	private:
		static void SerializeDependencies(Archive::Output& archive, Package::ContentsContainerType const& contents);
		static void SerializeContents(Archive::Output& archive, Package::ContentsContainerType const& contents);
	};

	struct PackageInput_Binary {
		using InfoTuple = std::tuple<StringID, Reflection::TypeInfoReference, std::span<std::byte const>>;

		std::vector<std::byte> bytes;
		//@todo This archive imposes a constraint that the getter methods should only ever be called once and in order.
		//      It would be better to create the archive inside those methods, so they can be called multiple times or in different orders.
		//      However, that requires better support for writing and skipping subsections in an archive.
		Archive::Input archive;

		PackageInput_Binary(std::istream& stream);

		std::unordered_set<StringID> GetDependencies();
		std::vector<InfoTuple> GetContentsInformation();
	};

	struct PackageOutput_YAML {
		YAML::Node root;

		PackageOutput_YAML(Package const& package);
		void Write(std::ostream& stream) const;

	private:
		static YAML::Node SerializeDependencies(Package::ContentsContainerType const& contents);
		static YAML::Node SerializeContents(Package::ContentsContainerType const& contents);
	};

	struct PackageInput_YAML {
		using InfoTuple = std::tuple<StringID, Reflection::TypeInfoReference, YAML::Node>;

		YAML::Node root;

		PackageInput_YAML(std::istream& stream);

		std::unordered_set<StringID> GetDependencies();
		std::vector<InfoTuple> GetContentsInformation();
	};

	using PackageOutput = std::variant<PackageOutput_Binary, PackageOutput_YAML>;
	using PackageInput = std::variant<PackageInput_Binary, PackageInput_YAML>;
}
