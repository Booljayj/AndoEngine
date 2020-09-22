#pragma once
#include "Containers/AllocatorFactories.h"
#include "Engine/STL.h"

/**
 * A tree structure that is stored in a linear array, using indices to define the tree.
 * Ideal for fast-iterating trees where the nodes are not moved after creation.
 */
template<typename NodeDataType, typename AllocatorFactory = DefaultAllocatorFactory>
struct FlatTree {
public:
	using IndexType = uint32_t;
	static constexpr IndexType None = static_cast<IndexType>(-1);

	struct Node {
		using DataType = NodeDataType;

		IndexType parent;
    	IndexType firstChild;
    	IndexType lastChild;
    	IndexType nextSibling;
    	IndexType prevSibling;

		DataType data;

		inline bool IsRoot() const { return parent == None; }
		inline bool IsLeaf() const { return firstChild == None; }
	};

	using Allocator = typename AllocatorFactory::template TAllocator<Node>;

	struct NodeHandle {
	private:
		FlatTree* tree = nullptr;
		IndexType index = None;

		NodeHandle(FlatTree* inTree, IndexType inIndex)
		: tree(inTree), index(inIndex)
		{}

	public:
		NodeHandle() = default;
		NodeHandle(const NodeHandle& other) = default;

		static const NodeHandle Invalid = NodeHandle{ nullptr, None };

		operator bool() const { return tree && tree->IsValidIndex(index); }

		typename Node::DataType const* Get() const { return bool(*this) ? &tree->nodes[index] : nullptr; }
		typename Node::DataType* Get() { return const_cast<typename Node::DataType*>(static_cast<const NodeHandle*>(this)->Get()); }

		typename Node::DataType const* operator->() const { return tree->nodes[index]; }
		typename Node::DataType* operator->() { return tree->nodes[index]; }

		NodeHandle Parent() const { return bool(*this) ? NodeHandle{tree, tree->nodes[index].parent} : NodeHandle::Invalid; }
		NodeHandle FirstChild() const { return bool(*this) ? NodeHandle{tree, tree->nodes[index].firstChild} : NodeHandle::Invalid; }
		NodeHandle LastChild() const { return bool(*this) ? NodeHandle{tree, tree->nodes[index].lastChild} : NodeHandle::Invalid; }
		NodeHandle NextSibling() const { return bool(*this) ? NodeHandle{tree, tree->nodes[index].nextSibling} : NodeHandle::Invalid; }
		NodeHandle PrevSibling() const { return bool(*this) ? NodeHandle{tree, tree->nodes[index].prevSibling} : NodeHandle::Invalid; }
	};

	FlatTree() = default;
	FlatTree(Allocator const& allocator)
	: nodes(allocator)
	{}

	inline IndexType Size() const { return nodeCount; }
	inline IndexType Capacity() const { return nodes.size(); }
	inline IndexType Slack() const { return nodes.size() - nodeCount; }

	void Reserve(IndexType size) {
		nodes.reserve(size);
	}

private:
	std::vector<Node, Allocator> nodes;
	IndexType nodeCount = 0;

	bool IsValidIndex(IndexType index) { return index < nodes.size(); }
};
