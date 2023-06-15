#pragma once
#include "Engine/Flags.h"
#include "Engine/Logging.h"
#include "Engine/Reflection.h"
#include "Engine/StandardTypes.h"

namespace Resources {
	DECLARE_LOG_CATEGORY(Resources);

	/** An identifier for a resource, which is globally unique and does not change over the lifetime of that resource */
	struct Identifier {
		REFLECT_ALIAS(Identifier);
		using ValueType = uint64_t;
		static constexpr ValueType MaxValue = std::numeric_limits<ValueType>::max();
		
		/** Generate a new random identifier. Thread-safe. */
		static Identifier Generate();

		constexpr Identifier() = default;
		constexpr Identifier(ValueType inValue) : value(inValue) {}

		inline bool operator==(const Identifier& other) const { return value == other.value; }
		inline bool operator!=(const Identifier& other) const { return !operator==(other); }

		/** Returns the internal value representation of this identifier */
		inline ValueType ToValue() const { return value; }

	private:
		ValueType value = 0;
	};
	
	/** Identifier which should never be used by a valid resource */
	constexpr Identifier ID_Invalid = Identifier{ Identifier::MaxValue };

	/** Flag values which can apply to a particular resource */
	enum class EResourceFlags : uint32_t {
		/** Indicates this resource has been fully initialized */
		Initialized,
	};
	using FResourceFlags = TFlags<EResourceFlags>;
}

REFLECT(Resources::Identifier, Alias);
