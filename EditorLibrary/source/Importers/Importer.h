#pragma once
#include "Engine/Logging.h"
#include "Engine/Core.h"
#include "Engine/Reflection.h"
#include "Engine/StringView.h"
#include "Resources/Resource.h"

namespace Importers {
	struct Importer {
	public:
		virtual bool Import(Resources::Resource& target, std::string_view source, std::string_view filename) = 0;

	protected:
		DECLARE_LOG_CATEGORY_MEMBER(Importer);
	};
}
