#pragma once
#include "Engine/Array.h"
#include "Engine/SmartPointers.h"
#include "Rendering/Vulkan/Resources.h"

namespace Rendering {
	/** Holds a collection of handles to various resources. Used to prevent those resources from being destroyed until it is safe to do so. */
	struct ResourcesCollection {
		ResourcesCollection() = default;
		ResourcesCollection(ResourcesCollection const&) = delete;
		ResourcesCollection(ResourcesCollection&&) = default;
		
		inline ResourcesCollection& operator=(ResourcesCollection&&) = default;

		/** Efficiently move all the handles from other into this collection. The original collection is cleared when doing this. */
		ResourcesCollection& operator<<(ResourcesCollection& other);

		/** Efficiently move a handle into this collection. The original handle is reset when doing this. */
		template<typename T>
		inline ResourcesCollection& operator<<(std::shared_ptr<T>& resources);
		/** Copy a handle into this collection.The original handle is not affected when doing this. */
		template<typename T>
		inline ResourcesCollection& operator+=(std::shared_ptr<T> const& resources);

		bool empty() const;
		void clear();
		void reserve(size_t num);

	private:
		std::vector<std::shared_ptr<GraphicsPipelineResources>> graphics_pipelines;
		std::vector<std::shared_ptr<MeshResources>> meshes;

		/** Utility to move each element of source into target, and clear source afterwards */
		template<typename T>
		void MoveAppend(std::vector<T>& target, std::vector<T>& source) {
			target.insert(target.end(), std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()));
			source.clear();
		}
	};

	template<> inline ResourcesCollection& ResourcesCollection::operator<< <GraphicsPipelineResources>(std::shared_ptr<GraphicsPipelineResources>& resources) { std::swap(graphics_pipelines.emplace_back(), resources); return *this; }
	template<> inline ResourcesCollection& ResourcesCollection::operator<< <MeshResources>(std::shared_ptr<MeshResources>& resources) { std::swap(meshes.emplace_back(), resources); return *this; }

	template<> inline ResourcesCollection& ResourcesCollection::operator+= <GraphicsPipelineResources>(std::shared_ptr<GraphicsPipelineResources> const& resources) { graphics_pipelines.emplace_back(resources); return *this; }
	template<> inline ResourcesCollection& ResourcesCollection::operator+= <MeshResources>(std::shared_ptr<MeshResources> const& resources) { meshes.emplace_back(resources); return *this; }
}
