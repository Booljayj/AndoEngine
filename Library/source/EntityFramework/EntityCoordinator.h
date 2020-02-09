#include <unordered_map>
#include <array>
#include <vector>
#include <typeinfo>
#include <tuple>
#include <string_view>
#include "Reflection/TypeInfo.h"

//============================================================
// Note: This file is a scratch space to test out different entity storage concepts.
//       None of this should be used directly in any way until that R&D work is complete.

struct ComponentInfo;

struct ComponentA { static constexpr size_t ID = 10; };
struct ComponentB { static constexpr size_t ID = 12; };
struct ComponentC { static constexpr size_t ID = 51213; };

struct MyArchetype {
	ComponentA a;
	ComponentB b;
	ComponentC c;

	static constexpr size_t ID = 1;
	static constexpr size_t ComponentIDs[] = { decltype(a)::ID, decltype(b)::ID, decltype(c)::ID };
};

struct BaseArchetypeContainer {};

template<typename ArchetypeType>
struct ArchetypeContainer : BaseArchetypeContainer {
	std::vector<ArchetypeType> archetypes;
};

template<typename ArchetypeType>
struct Entity {
	static constexpr size_t ID = 1;

	ArchetypeType* components;
};

struct EntityCoordinator {
	std::vector<std::tuple<size_t, std::unique_ptr<BaseArchetypeContainer>>> archetypeContainers;
};

//===

struct IArchetype {
	virtual std::vector<std::tuple<size_t, void*>> GetComponents() = 0;
};

template<typename... ComponentTypes>
struct TArchetype : IArchetype {
	std::tuple<ComponentTypes...> components;

	virtual
};

struct ArchetypeInfo {
	virtual size_t GetID() = 0;
	virtual std::basic_string_view<ComponentInfo const*> GetComponentInfos() = 0;
};

template< typename... ComponentTypes >
struct TArchetypeInfo {
	using ArchetypeType = std::tuple<ComponentTypes...>;
};

struct Archetype {
	std::vector<Reflection::TypeInfo const*> componentTypes;

	//@todo calculate this from the component types. Does not have to be persistent
	uint32_t uniqueID;
	std::vector<size_t> componentOffsets;
	size_t bufferSize;

	void Build() {
		//Start the buffer at location 0, which means every time this pointer is incremented the numeric value is equal to the offset.
		void* pointer = nullptr;
		//Start with a theoretically unlimited amount of space for the buffer. We won't use this value, the pointer is enough.
		size_t space = std::numeric_limits<size_t>::max();

		//For each component that this archetype defines, find space for it in some theoretical 16-byte aligned buffer
		for( Reflection::TypeInfo const* componentType : componentTypes ) {
			//Modify the pointer's current value to meet the component's alignment requirements.
			std::align( componentType->Definition.Alignment, componentType->Definition.Size, pointer, space );
			//Record the offset of this component in the buffer
			componentOffsets.push_back(reinterpret_cast<size_t>(pointer));
			//Advance the pointer to the next available location in the buffer
			pointer = static_cast<char*>(pointer) + componentType->Definition.Size;
		}
		//Record the total buffer size, which is equal to the final memory address offset
		bufferSize = reinterpret_cast<size_t>(pointer);
	}
};

template<class T, class Tuple>
struct tuple_index;

template<class T, class... Types>
struct tuple_index<T, std::tuple<T, Types...>> {
    static const size_t value = 0;
};

template<class T, class U, class... Types>
struct tuple_index<T, std::tuple<U, Types...>> {
    static const size_t value = 1 + tuple_index<T, std::tuple<Types...>>::value;
};

template<class T, class Tuple>
struct tuple_contains {
	constexpr bool value = std::false_type::value;
};

template<class T, class... Types>
struct tuple_contains<T, std::tuple<T, Types...>> {
	constexpr bool value = std::true_type::value;
};

template<class T, class U, class... Types>
struct tuple_contains<T, std::tuple<U, Types...>> {
	constexpr bool value = tuple_contains<T, std::tuple<Types...>>::value;
};

template<typename... ComponentTypes>
struct TArchetype {
	using StorageType = std::tuple<ComponentTypes...>;

	constexpr std::array<Reflection::TypeInfo const*, sizeof...( ComponentTypes )> ComponentTypeInfos = { Reflection::TypeResolver<ComponentTypes>::Get()... };

	template<typename... TREQUESTED_COMPS>
	static std::tuple<TREQUESTEDCOMPS*...> Get(StorageType& entity) {
		return std::make_tuple(&std::get<std::tuple_index<TREQUESTED_COMPS>>(entity)...);
	};
};

template<typename ArchetypeType>
struct TArchetypeContainer {
	using Archetype = ArchetypeType;

	std::vector<Archetype::StorageType> entities;
	std::vector<EntityID> entityIDs;
};
