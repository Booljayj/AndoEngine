#pragma once
#include "Engine/Core.h"
#include "Engine/GLM.h"

namespace Rendering {
	/** Compresses vec2 uv coordinates into a 32-bit unsigned integer value, as two half-precision floats. */
	struct CompressedUVs {
		CompressedUVs(glm::vec2 vec) : compressed(Compress(vec)) {}
		CompressedUVs(float u, float v) : CompressedUVs(glm::vec2(u, v)) {}
		inline CompressedUVs& operator=(glm::vec2 uv) { compressed = Compress(uv); return *this; }

	private:
		uint32_t compressed;
		static inline uint32_t Compress(glm::vec2 uv) { return glm::packHalf2x16(uv); }
	};

	/** Compresses a normalied vec3 into a 32-bit unsigned integer value, in standard 2_10_10_10 format. */
	struct CompressedNormal {
		CompressedNormal(glm::vec3 normal) : compressed(Compress(normal)) {}
		CompressedNormal(float x, float y, float z) : CompressedNormal(glm::vec3{ x, y, z }) {}
		inline CompressedNormal& operator=(glm::vec3 normal) { compressed = Compress(normal); return *this; }

	private:
		uint32_t compressed;

		static inline uint32_t Compress(glm::vec3 const normal) {
			//Pack using the 2_10_10_10 format, 10-bit precision on x, y, and z with an unused 2-bit w component.
			uint32_t const x = static_cast<uint32_t>(std::lroundf(std::clamp(normal.x, -1.0f, 1.0f) * 1023.0f));
			uint32_t const y = static_cast<uint32_t>(std::lroundf(std::clamp(normal.y, -1.0f, 1.0f) * 1023.0f));
			uint32_t const z = static_cast<uint32_t>(std::lroundf(std::clamp(normal.z, -1.0f, 1.0f) * 1023.0f));
			return (z << 20) | (y << 10) | x;
		}
	};
}
