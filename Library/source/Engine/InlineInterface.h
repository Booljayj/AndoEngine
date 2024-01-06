#pragma once
#include "Engine/StandardTypes.h"

template<typename BaseType>
struct TInlineClonable
{
	/** Clone this type into the destination pointer (via placement new) */
	virtual void Clone(BaseType* destination) const = 0;
};

/**
 * Holds an instance of a concrete class deriving from a virtual interface in inline memory.
 * Semantically, this acts as a value of type InterfaceType where methods are accessed with operator->,
 * but the internal type can be replaced by any interface implementation using Construct().
 */
template<typename InterfaceType, size_t MinConcreteTypeSize = 0>
	requires std::has_virtual_destructor_v<InterfaceType> and std::is_base_of_v<TInlineClonable<InterfaceType>, InterfaceType>
struct TInlineInterface
{
public:
	static constexpr size_t ConcreteTypeSize = std::max(sizeof(InterfaceType), MinConcreteTypeSize);
	
	template<typename ConcreteType>
	TInlineInterface(std::in_place_type_t<ConcreteType>) { ConstructValue<ConcreteType>(); }
	TInlineInterface(const TInlineInterface& other) { other->Clone(GetPointer<InterfaceType>()); }

	~TInlineInterface() { DestructValue(); }

	TInlineInterface& operator=(const TInlineInterface& other) {
		DestructValue();
		other->Clone(GetPointer<InterfaceType>());
		return *this;
	}

	const InterfaceType* operator->() const { return GetPointer<InterfaceType>(); }
	InterfaceType* operator->() { return GetPointer<InterfaceType>(); }

	/** Construct a new value of a concrete type into this instance */
	template<typename ConcreteType>
	void Construct() {
		DestructValue();
		ConstructValue<ConcreteType>();
	}

private:
	alignas(std::max_align_t) std::byte bytes[ConcreteTypeSize];

	template<typename ConcreteType>
	void ConstructValue() {
		static_assert(!std::is_abstract_v<ConcreteType>, "Concrete type must not be abstract");
		static_assert(std::is_base_of_v<InterfaceType, ConcreteType>, "Concrete type must derive from interface type");
		new (GetPointer<ConcreteType>()) ConcreteType();
	}

	void DestructValue() {
		GetPointer<InterfaceType>()->~InterfaceType();
	}
	
	template<typename ConcreteType>
	const ConcreteType* GetPointer() const {
		static_assert(sizeof(ConcreteType) <= ConcreteTypeSize, "Cannot use stack allocated interface type to construct a value larger than the available space");
		return reinterpret_cast<const ConcreteType*>(&bytes);
	}

	template<typename ConcreteType>
	ConcreteType* GetPointer() { return const_cast<ConcreteType*>(static_cast<const TInlineInterface*>(this)->GetPointer<ConcreteType>()); }
};
