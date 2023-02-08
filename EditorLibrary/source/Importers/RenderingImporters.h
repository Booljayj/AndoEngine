#pragma once
#include "Importers/Importer.h"
#include "Rendering/Shaders.h"
#include "shaderc/shaderc.hpp"

namespace Importers {
	struct ShaderImporter : public Importer {
	public:
		ShaderImporter();

		virtual bool Import(Resources::Resource& target, std::string_view source, std::string_view filename) override;

	protected:
		struct Includer : public shaderc::CompileOptions::IncluderInterface {
			virtual shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override;
			virtual void ReleaseInclude(shaderc_include_result* include) override;

		private:
			struct IncludeData {
				std::string name;
				std::string content;
			};
		};

		shaderc::CompileOptions options;
		shaderc::Compiler compiler;

		static Rendering::EShaderType GetFilenameType(std::string_view filename);
		static shaderc_shader_kind GetKind(Rendering::EShaderType type);
	};
}
