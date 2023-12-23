#include "Importers/RenderingImporters.h"
#include "Engine/Temporary.h"

namespace Importers {
	shaderc_include_result* ShaderImporter::Includer::GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) {
		shaderc_include_result* result = new shaderc_include_result;

		std::filesystem::path const path = std::filesystem::current_path() / "content" / "shaders" / requested_source;
		if (std::fstream file{ path, std::ios::in | std::ios::ate}) {
			size_t const size = file.tellg();
			file.seekg(0);

			IncludeData* data = new IncludeData();
			data->name = path.generic_string<t_string::value_type, t_string::traits_type, t_string::allocator_type>();
			data->content.resize(size, ' ');
			file.read(data->content.data(), size);

			result->source_name = data->name.data();
			result->source_name_length = data->name.size();
			result->content = data->content.data();
			result->content_length = data->content.size();
			result->user_data = data;

		} else {
			static std::string_view const message = "Could not open include file"sv;

			result->source_name = nullptr;
			result->source_name_length = 0;
			result->content = message.data();
			result->content_length = message.size();
			result->user_data = nullptr;
		}
		return result;
	}

	void ShaderImporter::Includer::ReleaseInclude(shaderc_include_result* include) {
		delete static_cast<IncludeData*>(include->user_data);
		delete include;
	}

	ShaderImporter::ShaderImporter() {
		options.SetIncluder(std::make_unique<Includer>());
	}

	bool ShaderImporter::Import(Resources::Resource& target, std::string_view source, std::string_view filename) {
		Rendering::Shader& shaderTarget = static_cast<Rendering::Shader&>(target);

		if (shaderTarget.GetShaderType() != GetFilenameType(filename)) return false;
		const shaderc_shader_kind kind = GetKind(shaderTarget.GetShaderType());

		//Preprocess the source
		const auto preprocessed = compiler.PreprocessGlsl(source.data(), source.size(), kind, filename.data(), options);
		if (preprocessed.GetCompilationStatus() != shaderc_compilation_status_success) {
			LOG(Importer, Error, "Preprocessor failure: {}", preprocessed.GetErrorMessage().data());
			return false;
		}

		//Compile the preprocessed source
		const char* preprocessedBegin = preprocessed.cbegin();
		const size_t preprocessedSize = std::distance(preprocessed.cbegin(), preprocessed.cend());
		const auto compiled = compiler.CompileGlslToSpv(preprocessedBegin, preprocessedSize, kind, filename.data(), options);
		if (compiled.GetCompilationStatus() != shaderc_compilation_status_success) {
			LOG(Importer, Error, "Compilation failure: {}", compiled.GetErrorMessage().data());
			return false;
		}

		LOG(Importer, Info, "Imported shader {}", filename.data());
		//Add the compiled bytecode to the resource
		shaderTarget.bytecode.clear();
		for (uint32_t word : compiled) shaderTarget.bytecode.push_back(word);
		return true;
	}

	Rendering::EShaderType ShaderImporter::GetFilenameType(std::string_view filename) {
		using namespace Rendering;
		if (filename.ends_with(".vert")) return EShaderType::Vertex;
		else if (filename.ends_with(".frag")) return EShaderType::Fragment;
		else return EShaderType::Vertex;
	}

	shaderc_shader_kind ShaderImporter::GetKind(Rendering::EShaderType type) {
		using namespace Rendering;
		switch (type) {
		case EShaderType::Vertex: return shaderc_vertex_shader;
		case EShaderType::Fragment: return shaderc_fragment_shader;
		default: return shaderc_vertex_shader;
		}
	}
}
