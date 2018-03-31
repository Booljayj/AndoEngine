#pragma once
#include <algorithm>
#include <array>
#include <tuple>
#include <vector>
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/Entity.h"

/** Handle used to quickly get a specific component type from an array of components using its local index. */
template<typename TCOMP>
struct TComponentHandle
{
private:
	size_t ComponentIndex;
public:
	TComponentHandle() = default;
	TComponentHandle( size_t InComponentIndex ) : ComponentIndex( InComponentIndex ) {}
	TComponentHandle<TCOMP>& operator=( TComponentHandle<TCOMP> const& Other ) = default;

	inline size_t GetIndex() const { return ComponentIndex; }
};

/** Base for entity filters, used by the EntityCollection to create and update a filter */
struct EntityFilterBase
{
	virtual ~EntityFilterBase() = default;
	virtual void Reserve( size_t Capacity ) = 0;
	virtual bool Add( EntityID const& ID, Entity const& E ) = 0;
	virtual bool Remove( EntityID const& ID ) = 0;
};

/** Entity filter that holds a collection of matching entities. It is managed by the EntityCollection that created it */
template<size_t SIZE>
struct EntityFilter : EntityFilterBase
{
	/** A match collected by the entity filter */
	struct FilterMatch
	{
		EntityID ID;
		std::array<void*, SIZE> Components;

		template<typename TCOMP>
		inline TCOMP* Get( TComponentHandle<TCOMP> const& Handle ) const { return static_cast<TCOMP*>( Components[Handle.GetIndex()] ); }
	};

private:
	std::array<ComponentTypeID, SIZE> TypeIDs;
	std::vector<FilterMatch> Matches;

public:
	EntityFilter( ComponentInfo const* (&Infos)[SIZE] )
	{
		for( size_t Index = 0; Index < SIZE; ++Index ) {
			TypeIDs[Index] = Infos[Index]->GetID();
		}
		std::sort( TypeIDs.begin(), TypeIDs.end() );
	}
	virtual ~EntityFilter() = default;

	void Reserve( size_t Capacity ) override { Matches.reserve( Capacity ); }

	bool Add( EntityID const& ID, Entity const& E ) override
	{
		bool const HasAllComponents = E.HasAll( TypeIDs.begin(), TypeIDs.size() );
		if( HasAllComponents ) {
			FilterMatch NewMatch;
			NewMatch.ID = ID;
			for( size_t Index = 0; Index < SIZE; ++Index ) {
				NewMatch.Components[Index] = E.Get( TypeIDs[Index] );
			}
			Matches.push_back( NewMatch );
			return true;
		}
		return false;
	}

	bool Remove( EntityID const& ID ) override
	{
		size_t RemovedCount = 0;
		for( typename std::vector<FilterMatch>::reverse_iterator Iter = Matches.rbegin(); Iter != Matches.rend(); ++Iter ) {
			if( Iter->ID == ID ) {
				std::swap( *Iter, Matches.back() );
				Matches.pop_back();
				++RemovedCount;
			}
		}
		return RemovedCount > 0;
	}

	template<typename TCOMP>
	TComponentHandle<TCOMP> GetMatchComponentHandle( TComponentInfo<TCOMP> const* Info ) const
	{
		size_t const Index = std::distance( TypeIDs.begin(), std::find( TypeIDs.begin(), TypeIDs.end(), Info->GetID() ) );
		return TComponentHandle<TCOMP>{ Index };
	}

	typename std::vector<FilterMatch>::const_iterator begin() const { return Matches.begin(); }
	typename std::vector<FilterMatch>::const_iterator end() const { return Matches.end(); }
};
