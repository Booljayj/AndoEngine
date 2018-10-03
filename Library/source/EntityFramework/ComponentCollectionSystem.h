#pragma once
#include <vector>
#include "Engine/Context.h"
#include "EntityFramework/Types.h"

struct ComponentInfo;

/** Manages a collection of components, and can be used for fast queries that provide component information */
struct ComponentCollectionSystem
{
public:
	/** A searcher is a kind of forward iterator that returns sequential infos from a collection by feeding in sorted type IDs one at a time. */
	struct Searcher {
		Searcher( ComponentCollectionSystem const& InCollection );

		/** Get the current component that was found. Do not call on an invalid searcher. */
		ComponentInfo const* Get() const;
		/** Find the next sequential type ID. Returns false if the end of the registered IDs was reached and the iterator is no longer valid. */
		bool Next( ComponentTypeID NextTypeID );
		/** Reset back to the initial state */
		void Reset();

	private:
		ComponentCollectionSystem const& Collection;
		std::vector<ComponentTypeID>::const_iterator CurrentIter;
	};

	bool Startup( CTX_ARG, ComponentInfo const* const* Infos, size_t Count );
	bool Shutdown( CTX_ARG );

	/** Returns true if all the components in the array are part of this collection. */
	bool ContainsComponentInfos( CTX_ARG, ComponentInfo const* const* Infos, size_t Count ) const;

	/** Find the component info that corresponds to the type ID */
	ComponentInfo const* GetComponentInfo( CTX_ARG, ComponentTypeID TypeID ) const;
	/** Find the component info that corresponds to the name */
	ComponentInfo const* GetComponentInfo( CTX_ARG, char const* Name ) const;

	/** Fill the output array with the infos that correspond to the type IDs */
	void GetComponentInfos( CTX_ARG, ComponentTypeID const* TypeIDs, ComponentInfo const** OutInfos, size_t Count ) const;
	/** Fill the output array with the infos that correspond to the names */
	void GetComponentInfos( CTX_ARG, char const* const* Names, ComponentInfo const** OutInfos, size_t Count ) const;

private:
	std::vector<ComponentInfo const*> RegisteredInfos;
	std::vector<ComponentTypeID> RegisteredTypeIDs;
	std::vector<char const*> RegisteredNames;
};
