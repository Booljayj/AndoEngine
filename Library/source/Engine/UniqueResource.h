#pragma once
#include "Engine/StandardTypes.h"

/**
 * A deleter object that can delete a resource owned by a TUniqueResource instance.
 * Also provides information on the resource value that should be considered "null", meaning no resource is being held.
 */
template<typename T, typename ResourceType>
concept UniqueResourceDeleter =
	std::copyable<T> and std::copyable<ResourceType> and
	requires (T t, ResourceType r) {
		{ t.operator()(r) } -> std::convertible_to<void>;
		{ T::NullResourceValue } -> std::convertible_to<ResourceType>;
	};

template<typename T, typename ResourceType>
concept UniqueResourceSpanDeleter =
std::copyable<T> and std::copyable<ResourceType> and
	requires (T t, std::span<ResourceType const> s) {
		{ t.operator()(s) } -> std::convertible_to<void>;
		{ T::NullResourceValue } -> std::convertible_to<ResourceType>;
};

/**
 * Owns a resource and will destroy it when going out of scope. The owned resource is optional, and nothing is done if there is no owned resource.
 * The resource can be released to invalidate the TUniqueResource instance and transfer ownership to something else.
 */
template<std::copyable ResourceType, UniqueResourceDeleter<ResourceType> DeleterType>
struct TUniqueResource {
	TUniqueResource(ResourceType resource, DeleterType const& deleter = DeleterType{}) : resource(resource), deleter(deleter) {}
	TUniqueResource(DeleterType const& deleter = DeleterType{}) : deleter(deleter) {}

	TUniqueResource(TUniqueResource const&) = delete;
	TUniqueResource(TUniqueResource&& other) : resource(other.resource), deleter(other.deleter) {
		other.resource = DeleterType::NullResourceValue;
	}

	~TUniqueResource() {
		if (resource != DeleterType::NullResourceValue) deleter(resource);
	}

	inline operator bool() const { return resource == DeleterType::NullResourceValue; }
	inline operator ResourceType() const { return resource; }
	
	TUniqueResource& operator=(TUniqueResource&& other) {
		std::swap(resource, other.resource);
		std::swap(deleter, other.deleter);
	}

	ResourceType& Get() { return resource; }
	ResourceType const& Get() const { return resource; }

	ResourceType Release() {
		ResourceType const copy = resource;
		resource = DeleterType::NullResourceValue;
		return copy;
	}

	void Reset(ResourceType newResource = DeleterType::NullResourceValue) {
		if (resource != DeleterType::NullResourceValue) {
			deleter(resource);
		}
		resource = newResource;
	}

private:
	ResourceType resource = DeleterType::NullResourceValue;
	DeleterType deleter;
};

/**
 * Special kind of TUniqueResource that keeps track of a collection of resources that are treated as a single unit.
 * The individual resources are either all valid, or all invalid, never a mix of the two.
 */
template<std::copyable ResourceType, size_t Size, UniqueResourceSpanDeleter<ResourceType> DeleterType>
	requires (Size > 0)
struct TUniqueResources {
	TUniqueResources(std::array<ResourceType, Size> resources, DeleterType const& deleter = DeleterType{}) : resources(resources), deleter(deleter) {}
	TUniqueResources(DeleterType const& deleter = DeleterType{}) : deleter(deleter) {
		resources.fill(DeleterType::NullResourceValue);
	}

	TUniqueResources(TUniqueResources const&) = delete;
	TUniqueResources(TUniqueResources&& other) : resources(other.resources), deleter(other.deleter) {
		other.resources.fill(DeleterType::NullResourceValue);
	}

	~TUniqueResources() {
		if (resources[0] != DeleterType::NullResourceValue) { deleter(resources); }
	}

	inline operator bool() const { return resources[0] == DeleterType::NullResourceValue; }
	
	TUniqueResources& operator=(TUniqueResources&& other) {
		std::swap(resources, other.resources);
		std::swap(deleter, other.deleter);
	}

	ResourceType& operator[](size_t index) { return resources[index]; }
	ResourceType const& operator[](size_t index) const { return resources[index]; }

	auto data() const { return resources.data(); }
	auto size() const { return resources.size(); }
	auto begin() const { return resources.begin(); }
	auto end() const { return resources.end(); }
	
	std::array<ResourceType, Size> Release() {
		std::array<ResourceType, Size> const copy = resources;
		resources.fill(DeleterType::NullResourceValue);
		return copy;
	}

	void Reset() {
		if (resources[0] != DeleterType::NullResourceValue) deleter(resources);
		resources.fill(DeleterType::NullResourceValue);
	}
	void Reset(std::array<ResourceType, Size> newResources) {
		if (resources[0] != DeleterType::NullResourceValue) deleter(resources);
		resources = newResources;
	}

private:
	std::array<ResourceType, Size> resources;
	DeleterType deleter;
};

template<std::copyable ResourceType, UniqueResourceDeleter<ResourceType> DeleterType>
TUniqueResource<ResourceType, DeleterType> MakeUniqueResource(ResourceType resource, DeleterType const& deleter = DeleterType{}) {
	return TUniqueResource<ResourceType, DeleterType>{ resource, deleter };
}
