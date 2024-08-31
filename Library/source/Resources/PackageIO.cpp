#include "Resources/PackageIO.h"
#include "Engine/Algo.h"
#include "Resources/Package.h"
#include "Resources/Streaming.h"
#include "Resources/StreamingUtils.h"

namespace Resources {
	//=================================================================================
	//Binary package format is as follows, where the elements in the buffer are specified as [Name:Size]:
	//[DependencyCount:sizeof(size_t)][Dependencies:DependencyCount]
	//[ResourceAName:sizeof(StringID)][ResourceAType:sizeof(TypeInfoReference)][ResourceADataSize:sizeof(size_t)][ResourceAData:ResourceADataSize]
	//[ResourceBName:sizeof(StringID)][ResourceBType:sizeof(TypeInfoReference)][ResourceBDataSize:sizeof(size_t)][ResourceBData:ResourceBDataSize]
	//...

	PackageOutput_Binary::PackageOutput_Binary(Package const& package) {
		Archive::Output archive{ bytes };

		//Copy the contents, then serialize. This means further changes during serialization will not be included, but avoids locking the package for a long duration.
		auto const contents = *package.GetContentsView();
		SerializeDependencies(archive, contents);
		SerializeContents(archive, contents);
	}

	void PackageOutput_Binary::Write(std::ostream& stream) const {
		std::span<char const> const characters = stdext::from_bytes<char>(bytes);
		stream.write(characters.data(), characters.size());
	}

	void PackageOutput_Binary::SerializeDependencies(Archive::Output& archive, Package::ContentsContainerType const& contents) {
		auto const dependencies = GatherPackageDependencies(contents);
		archive << dependencies;
	}

	void  PackageOutput_Binary::SerializeContents(Archive::Output& archive, Package::ContentsContainerType const& contents) {
		archive << contents.size();

		std::vector<std::byte> resource_bytes;
		for (auto const& pair : contents) {
			StringID const name = pair.first;
			Resources::Resource const& resource = *pair.second;

			Reflection::StructTypeInfo const& type = resource.GetTypeInfo();
			{
				//@todo We don't need a temporary vector here if we can write a "section" to the output, where the size is written before the bytes following it.
				resource_bytes.clear();
				Archive::Output resource_archive{ resource_bytes };
				type.Serialize(resource_archive, &resource);
			}

			archive << name << Reflection::TypeInfoReference{ type } << resource_bytes;
		}
	}

	PackageInput_Binary::PackageInput_Binary(std::istream& stream)
		: bytes(Utility::ExtractBytes(stream))
		, archive(bytes)
	{}

	std::unordered_set<StringID> PackageInput_Binary::GetDependencies() {
		std::unordered_set<StringID> packages;
		archive >> packages;

		packages.erase(StringID::None);
		packages.erase(StringID::Temporary);

		return packages;
	}

	std::vector<PackageInput_Binary::InfoTuple> PackageInput_Binary::GetContentsInformation() {
		size_t count = 0;
		archive >> count;

		std::vector<InfoTuple> results;
		results.reserve(count);

		for (size_t index = 0; index < count; ++index) {
			StringID id = StringID::None;
			Reflection::TypeInfoReference type_reference;
			std::span<std::byte const> buffer;

			archive >> id >> type_reference >> buffer;
			results.emplace_back(id, type_reference, buffer);
		}

		return results;
	}

	//=================================================================================
	//YAML package format is as follows, where this is a rough example of what the YAML file looks like when shown in a text editor:
	//- dependencies: [DependencyA, DependencyB, DependencyC, ...]
	//- contents:
	//    - name: ResourceAName
	//      type: ... #TypeInfoReference
	//      object:
	//        - Property1: ...
	//        - Property2: ...
	//        # etc
	//    - name: ResourceBName
	//      type: ...
	//      object:
	//        - Property1: ...
	//        - Property2: ...
	//        # etc

	static std::string_view const dependencies_name = "dependencies"sv;
	static std::string_view const contents_name = "contents"sv;
	static std::string_view const name_name = "name"sv;
	static std::string_view const type_name = "type"sv;
	static std::string_view const object_name = "object"sv;

	PackageOutput_YAML::PackageOutput_YAML(Package const& package)
		: root(YAML::NodeType::Map)
	{
		//Copy the contents, then serialize. This means further changes during serialization will not be changed, but avoids locking the package for a long duration.
		auto const contents = *package.GetContentsView();
		root[dependencies_name] = SerializeDependencies(contents);
		root[contents_name] = SerializeContents(contents);
	}

	void PackageOutput_YAML::Write(std::ostream& stream) const {
		stream << root;
	}

	YAML::Node PackageOutput_YAML::SerializeDependencies(Package::ContentsContainerType const& contents) {
		using namespace YAML;

		Node sequence{ NodeType::Sequence };
		for (StringID const dependency : GatherPackageDependencies(contents)) sequence.push_back(dependency);
		return sequence;
	}

	YAML::Node PackageOutput_YAML::SerializeContents(Package::ContentsContainerType const& contents) {
		using namespace YAML;

		Node sequence{ NodeType::Sequence };
		for (auto const& pair : contents) {
			StringID const name = pair.first;
			Resources::Resource const& resource = *pair.second;
			Reflection::StructTypeInfo const& type = resource.GetTypeInfo();

			Node resource_node{ NodeType::Map };
			resource_node[name_name] = name.ToStringView();
			resource_node[type_name] = Reflection::TypeInfoReference{ type };
			resource_node[object_name] = type.Serialize(&resource);

			sequence.push_back(resource_node);
		}
		return sequence;
	}

	PackageInput_YAML::PackageInput_YAML(std::istream& stream)
		: root(YAML::Load(stream))
	{}

	std::unordered_set<StringID> PackageInput_YAML::GetDependencies() {
		std::unordered_set<StringID> packages;
		for (YAML::Node const node : root[dependencies_name]) packages.emplace(node.as<StringID>());

		packages.erase(StringID::None);
		packages.erase(StringID::Temporary);
		
		return packages;
	}

	std::vector<PackageInput_YAML::InfoTuple> PackageInput_YAML::GetContentsInformation() {
		std::vector<InfoTuple> results;

		YAML::Node const sequence = root[contents_name];
		for (YAML::Node const resource_node : sequence) {
			results.emplace_back(
				resource_node[name_name].as<StringID>(),
				resource_node[type_name].as<Reflection::TypeInfoReference>(),
				resource_node[object_name]
			);
		}

		return results;
	}
}
