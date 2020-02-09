#pragma once
#include <vector>
#include <array>
#include <bitset>
#include <memory>
#include "EntityFramework/Internal/ManagedComponentBlock.h"
#include "EntityFramework/ComponentManager.h"
#include "EntityFramework/Serializer.h"

/** A component manager which allocates fixed-size component chunks dynamically as needed and uses standardized manipulation */
template<class ComponentType, size_t BlockSize>
struct TChunkedComponentManager : public ComponentManager {
	using BlockType = TInlineManagedComponentBlock<ComponentType, BlockSize>;

protected:
	size_t lowestFreeBlockIndex = 0;
	std::vector<BlockType*> blocks;

public:
	TChunkedComponentManager() = default;
	virtual ~TChunkedComponentManager() {
		for (BlockType* blockPtr : blocks) {
			delete blockPtr;
		}
	}

	static ComponentType* Cast(ptr_t comp) { return static_cast<ComponentType*>( comp ); }
	static ComponentType const* Cast(cptr_t comp) { return static_cast<ComponentType const*>( comp ); }

	virtual void Setup( Entity const& newEntity, ptr_t newComponent ) const override {}

	virtual ptr_t Retain() override {
		//Grow the number of managed blocks if we don't currently have any free components
		if (lowestFreeBlockIndex == blocks.size()) {
			blocks.emplace_back(new BlockType());
		}
		//Get an unused component from a block and reinitialize it.
		ComponentType* retainedComponent = blocks[lowestFreeBlockIndex]->Retain();
		new(retainedComponent) ComponentType{};
		//Seek ahead if we've run out of free components in the lowest block
		while (lowestFreeBlockIndex < blocks.size() && !blocks[lowestFreeBlockIndex]->HasAnyFree()) {
			++lowestFreeBlockIndex;
		}
		return retainedComponent;
	}

	virtual void Release(ptr_t rawReleasedComponent) override {
		ComponentType* releasedComponent = Cast(rawReleasedComponent);
		for( size_t containingBlockIndex = 0; containingBlockIndex < blocks.size(); ++containingBlockIndex ) {
			BlockType* block = blocks[containingBlockIndex];
			if (block->Contains(releasedComponent)) {
				block->Release(releasedComponent);
				if (containingBlockIndex < lowestFreeBlockIndex) {
					lowestFreeBlockIndex = containingBlockIndex;
				}
				return;
			}
		}
		assert(false); //Should never reach this point unless the component was not part of this manager
	}

	virtual void Save(cptr_t comp, ByteStream& bytes) override { Serializer<ComponentType>::Save(*Cast(comp), bytes); }
	virtual void Load(ptr_t comp, ByteStream const& bytes) override { Serializer<ComponentType>::Load(*Cast(comp), bytes); }
	virtual void Copy(cptr_t compA, ptr_t compB) override { *Cast(compB) = *Cast(compA); }
	virtual void Wipe(ptr_t comp) override { new (comp) ComponentType{}; }

	size_t CountTotal() const override final {
		return BlockSize * blocks.size();
	}

	size_t CountFree() const override final {
		return CountTotal() - CountUsed();
	}

	size_t CountUsed() const override final {
		size_t runningTotal = 0;
		for( size_t index = 0; index < blocks.size(); ++index ) {
			runningTotal += blocks[index]->CountUsed();
		}
		return runningTotal;
	}

	template<typename PredicateType>
	void ForEach(PredicateType const& predicate) const {
		for (BlockType* block : blocks) {
			for (ComponentType& comp : *block) {
				if (block->IsUsed(&comp)) predicate(&comp);
			}
		}
	}

	template<typename PredicateType>
	void ForEachAll(PredicateType const& predicate) const {
		for (std::unique_ptr<BlockType>& block : blocks) {
			for (ComponentType& comp : *block) {
				predicate(&comp);
			}
		}
	}
};
