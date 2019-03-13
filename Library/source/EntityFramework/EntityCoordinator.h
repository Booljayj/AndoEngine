#include <unordered_map>
#include <array>
#include <vector>
#include <typeinfo>
#include <tuple>
#include <string_view>
#include "Reflection/TypeInfo.h"

struct ComponentInfo;

struct ComponentA { static constexpr size_t ID = 10; };
struct ComponentB { static constexpr size_t ID = 12; };
struct ComponentC { static constexpr size_t ID = 51213; };

struct MyArchetype {
	ComponentA A;
	ComponentB B;
	ComponentC C;

	static constexpr size_t ID = 1;
	static constexpr size_t ComponentIDs[] = { decltype( A )::ID, decltype( B )::ID, decltype( C )::ID };
};

struct BaseArchetypeContainer {};

template<typename TARCHETYPE>
struct ArchetypeContainer : BaseArchetypeContainer {
	std::vector<TARCHETYPE> Archetypes;
};

template<typename TARCHETYPE>
struct Entity {
	static constexpr size_t ID = 1;

	TARCHETYPE* Components;
};

struct EntityCoordinator {
	std::vector<std::tuple<size_t, std::unique_ptr<BaseArchetypeContainer>>> ArchetypeContainers;
};

//===

struct IArchetype {
	virtual std::vector<std::tuple<size_t, void*>> GetComponents() = 0;
};

template<typename... TCOMPS>
struct TArchetype : IArchetype {
	std::tuple<TCOMPS...> Components;

	virtual
};

struct ArchetypeInfo {
	virtual size_t GetID() = 0;
	virtual std::basic_string_view<ComponentInfo const*> GetComponentInfos() = 0;
};

template< typename... TCOMPS >
struct TArchetypeInfo {
	using TYPE = std::tuple<TCOMPS...>;
};

struct Archetype
{
	std::vector<Reflection::TypeInfo const*> ComponentTypes;

	//@todo calculate this from the component types. Does not have to be persistent
	uint32_t UniqueID;
	std::vector<size_t> ComponentOffsets;
	size_t BufferSize;

	void Build() {
		//Start the buffer at location 0, which means every time this pointer is incremented the numeric value is equal to the offset.
		void* Pointer = nullptr;
		//Start with a theoretically unlimited amount of space for the buffer. We won't use this value, the pointer is enough.
		size_t Space = std::numeric_limits<size_t>::max();

		//For each component that this archetype defines, find space for it in some theoretical 16-byte aligned buffer
		for( Reflection::TypeInfo const* ComponentType : ComponentTypes ) {
			//Modify the pointer's current value to meet the component's alignment requirements.
			std::align( ComponentType->Alignment, ComponentType->Size, Pointer, Space );
			//Record the offset of this component in the buffer
			ComponentOffsets.push_back( reinterpret_cast<size_t>( Pointer ) );
			//Advance the pointer to the next available location in the buffer
			Pointer = static_cast<char*>( Pointer ) + ComponentType->Size;
		}
		//Record the total buffer size, which is equal to the final memory address offset
		BufferSize = reinterpret_cast<size_t>( Pointer );
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

template<typename... TCOMPS>
struct TArchetype {
	using StorageType = std::tuple<TCOMPS...>;

	constexpr std::array<Reflection::TypeInfo const*, sizeof...( TCOMPS )> ComponentTypeInfos = { Reflection::TypeResolver<TCOMPS>::Get()... };

	template<typename... TREQUESTED_COMPS>
	static std::tuple<TREQUESTEDCOMPS*...> Get( StorageType& Entity ) {
		return std::make_tuple( &std::get<std::tuple_index<TREQUESTED_COMPS>>( Entity )... );
	};
};

template<typename TARCHETYPE>
struct TArchetypeContainer {
	using Archetype = TARCHETYPE;

	std::vector<Archetype::StorageType> Entities;
	std::vector<EntityID> EntityIDs;
};
