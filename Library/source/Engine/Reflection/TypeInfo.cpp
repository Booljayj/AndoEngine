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

	std::deque<TypeInfo const*> RegisteredTypeInfo::infos;

	RegisteredTypeInfo::RegisteredTypeInfo(TypeInfo const& info) : cached(&info) {
		infos.push_back(cached);
	}

	RegisteredTypeInfo::~RegisteredTypeInfo() {
		const auto iter = ranges::find(infos, cached);
		if (iter != infos.end()) {
			//Perform a swap-remove to improve performance when many types are registered.
			const auto last = infos.end() - 1;

			if (iter == last) infos.pop_back();
			else {
				std::iter_swap(iter, last);
				infos.pop_back();
			}
		}
	}
}
