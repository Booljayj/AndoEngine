#include "Engine/Reflection/TypeInfo.h"
#include "Engine/Algo.h"

namespace Reflection {
	void TypeUniquePointerDeleter::operator()(void* pointer) {
		std::free(pointer);
	}

	TypeUniquePointer TypeInfo::Allocate() const {
		void* pointer = std::malloc(memory.size);
		return TypeUniquePointer{ static_cast<std::byte*>(pointer) };
	}
}
