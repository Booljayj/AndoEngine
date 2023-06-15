#include "Resources/ResourceTypes.h"

namespace Resources {
	DEFINE_LOG_CATEGORY(Resources, Info);

	Identifier Identifier::Generate() {
		thread_local std::mt19937_64 random = std::mt19937_64{ std::random_device{}() };
		static_assert(std::is_same_v<decltype(random)::result_type, ValueType>, "Random generator result type should match the value type");

		//The upper range of generation is 1 less than the maximum. This will make it impossible to generate the invalid id.
		std::uniform_int_distribution<uint64_t> dist{ 0, MaxValue - 1 };
		return dist(random);
	}
}

DEFINE_REFLECT_ALIAS(Resources, Identifier);
