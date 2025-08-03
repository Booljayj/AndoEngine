#pragma once
#include "Engine/Core.h"
#include "Engine/GLM.h"

namespace Rendering {
	template<size_t Size>
	struct CompressedVec {};

	/** Compresses a vec2 into a 32-bit unsigned integer value, as two half-precision floats. */
	template<>
	struct CompressedVec<2> {
		CompressedVec(glm::vec2 vec) : compressed(Compress(vec)) {}
		CompressedVec(float u, float v) : CompressedVec(glm::vec2(u, v)) {}
		inline CompressedVec& operator=(glm::vec2 uv) { compressed = Compress(uv); return *this; }

	private:
		uint32_t compressed;
		static inline uint32_t Compress(glm::vec2 uv) { return glm::packHalf2x16(uv); }
	};
	using CompressedVec2 = CompressedVec<2>;

	/** Compresses a vec3 into a 32-bit unsigned integer value, in standard 2_10_10_10 format. */
	template<>
	struct CompressedVec<3> {
		CompressedVec(glm::vec3 vec) : compressed(Compress(vec)) {}
		CompressedVec(float x, float y, float z) : CompressedVec(glm::vec3(x, y, z)) {}
		inline CompressedVec& operator=(glm::vec3 vec) { compressed = Compress(vec); return *this; }

	private:
		uint32_t compressed;
		static inline uint32_t Compress(glm::vec3 vec) {
			//Pack using the 2_10_10_10 format, 10-bit precision on x, y, and z with an unused 2-bit w component.
			uint32_t const x = static_cast<uint32_t>(std::lroundf(std::clamp(vec.x, -1.0f, 1.0f) * 1023.0f));
			uint32_t const y = static_cast<uint32_t>(std::lroundf(std::clamp(vec.y, -1.0f, 1.0f) * 1023.0f));
			uint32_t const z = static_cast<uint32_t>(std::lroundf(std::clamp(vec.z, -1.0f, 1.0f) * 1023.0f));
			return (z << 20) | (y << 10) | x;
		}
	};
	using CompressedVec3 = CompressedVec<3>;
}
