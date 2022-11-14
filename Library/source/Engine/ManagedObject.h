#pragma once
#include "Engine/StandardTypes.h"

/**
 * A managed object maintains an internal thread-safe count of how many things are referencing it.
 * References are managed with Handles, which modify the internal reference count.
 * When the reference count of a managed object is 0, nothing is referencing it. It is safe to
 * destroy as long as no new handles can be created during destruction.
 */
struct ManagedObject {
private:
	struct HandleBase {
	public:
		inline operator bool() const { return !!object; }

	protected:
		ManagedObject* object = nullptr;

		HandleBase() = default;
		HandleBase(HandleBase const& other) : object(other.object) {
			if (object) object->refCount.fetch_add(1);
		}
		HandleBase(HandleBase&& other) : object(other.object) {
			other.object = nullptr;
		}
		HandleBase(ManagedObject& inObject) : object(&inObject) {
			object->refCount.fetch_add(1);
		}

		~HandleBase() {
			if (object) object->refCount.fetch_sub(1);
		}

		inline void Swap(HandleBase& other) { std::swap(object, other.object); }
	};

public:
	/** A handle to a managed object. Instances of handles will increase the reference count for the managed object. */
	template<typename InObjectType>
	struct Handle : public HandleBase {
		using ObjectType = InObjectType;
		using MutableObjectType = std::remove_const_t<ObjectType>;
		using HandleBase::operator bool;

		/** A factory which is allowed to create handles from a raw object reference. It is assumed to be the instance managing the raw object references. */
		struct Factory {
		protected:
			/** Create a new handle from a raw object reference. Must only be called in a thread-safe context. */
			static Handle<ObjectType> CreateHandle(ObjectType& object) {
				return Handle<ObjectType>{ object };
			}
		};

		static_assert(!std::is_pointer_v<ObjectType>, "Handles cannot hold an object which is a pointer");
		static_assert(std::is_base_of_v<ManagedObject, ObjectType>, "Handle template parameter must derive from ManagedObject");
		template<typename A, typename B> friend Handle<A> Cast(Handle<B> const&);
		template<typename A, typename B> friend Handle<A> Cast(Handle<B>&&);

		Handle() noexcept = default;
		Handle(std::nullptr_t) noexcept : HandleBase() {}

		template<typename OtherObjectType>
		Handle(Handle<OtherObjectType> const& other) noexcept : HandleBase(other) {}
		template<typename OtherObjectType>
		Handle(Handle<OtherObjectType>&& other) noexcept : HandleBase(std::move(other)) {}

		template<typename OtherObjectType>
		Handle& operator=(Handle<OtherObjectType> const& other) { Handle(other).Swap(*this); }
		template<typename OtherObjectType>
		Handle& operator=(Handle<OtherObjectType>&& other) { Handle(std::move(other)).Swap(*this); }

		inline ObjectType* operator->() const { return static_cast<ObjectType*>(object); }
		inline ObjectType& operator*() const { return *static_cast<ObjectType*>(object); }

		inline ObjectType* Get() const { return static_cast<ObjectType*>(object); }
		inline void Reset() { Handle().Swap(*this); }
		inline void Swap(Handle& other) { HandleBase::Swap(other); }

	private:
		Handle(MutableObjectType& inObject) noexcept : HandleBase(inObject) {}
	};

	/** Get the current number of references to this object. Thread-safe. */
	size_t GetRefCount() const noexcept { return refCount.load(); }

private:
	std::atomic<size_t> refCount;
};

static_assert(std::is_trivially_destructible_v<ManagedObject>, "ManagedObject base class should be trivially destructible");

/** Create a copy of a handle cast to a different type */
template<typename OtherObjectType, typename ObjectType>
ManagedObject::Handle<OtherObjectType> Cast(ManagedObject::Handle<ObjectType> const& handle) {
	if (handle.object) return ManagedObject::Handle<OtherObjectType>{ *static_cast<OtherObjectType*>(handle.object) };
	else return nullptr;
}

/** Convert a handle to a different type */
template<typename OtherObjectType, typename ObjectType>
ManagedObject::Handle<OtherObjectType> Cast(ManagedObject::Handle<ObjectType>&& handle) {
	ManagedObject::Handle<OtherObjectType> other;
	other.object = static_cast<OtherObjectType*>(handle.object);
	handle.object = nullptr;
	return other;
}
