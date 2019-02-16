#include <cstdint>
#include <typeinfo>

namespace Reflection {
	/** Type information that is specified by the compiler */
	struct CompilerDefinition {
		/** The size in bytes of an instance of this type */
		size_t Size = 0;
		/** The alignment in bytes of an instance of this type */
		size_t Alignment = 0;
		/** The compiler-generated name of this type. Not expected to be human-readable or stable between compilations. */
		const char* MangledName = nullptr;
	};

	template<typename T>
	inline CompilerDefinition GetCompilerDefinition() {
		return CompilerDefinition{ sizeof( T ), alignof( T ), typeid( T ).name() };
	}
	template<>
	inline CompilerDefinition GetCompilerDefinition<void>() {
		return CompilerDefinition{ 0, 0, typeid( void ).name() };
	}
}