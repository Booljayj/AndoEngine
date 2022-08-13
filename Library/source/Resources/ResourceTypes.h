#pragma once
#include "Engine/Flags.h"
#include "Engine/Logging/LogCategory.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"

namespace Resources {
	DECLARE_LOG_CATEGORY(Resources);

	/** An identifier for a resource, which is globally unique and does not change over the lifetime of that resource */
	struct Identifier {
		REFLECT_ALIAS(Identifier);

		using ValueType = uint64_t;

		/** Generate a new random identifier */
		inline static Identifier Generate() {
			return Identifier{ static_cast<ValueType>(std::rand()) << 32 | static_cast<ValueType>(std::rand()) };
		}

		Identifier() = default;
		Identifier(ValueType inValue) : value(inValue) {}

		inline bool operator==(const Identifier& other) const { return value == other.value; }
		inline bool operator!=(const Identifier& other) const { return !operator==(other); }

		/** Returns the internal value representation of this identifier */
		inline ValueType ToValue() const { return value; }

	private:
		ValueType value = 0;
	};

	/** Flag values which can apply to a particular resource */
	enum class EResourceFlags : uint32_t {
		/** Indicates this resource has been fully initialized */
		Initialized,
	};
	using FResourceFlags = TFlags<EResourceFlags>;
}

REFLECT(Resources::Identifier, Alias);
