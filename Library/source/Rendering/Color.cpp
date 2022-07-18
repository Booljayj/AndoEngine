#include "Rendering/Color.h"
#include "Reflection/StandardResolvers.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	const TStructTypeInfo<Rendering::Color> info_Color = TStructTypeInfo<Rendering::Color>{}
		.Description("4-component color"sv)
		.Variables({
			VariableInfo{ &Rendering::Color::r, "r"sv, nullptr, FVariableFlags::None },
			VariableInfo{ &Rendering::Color::g, "g"sv, nullptr, FVariableFlags::None },
			VariableInfo{ &Rendering::Color::b, "b"sv, nullptr, FVariableFlags::None },
			VariableInfo{ &Rendering::Color::a, "a"sv, nullptr, FVariableFlags::None }
		});

	TypeInfo const* TypeResolver<Rendering::Color>::Get() { return &info_Color; }
}
