#include "Engine/StandardTypes.h"

/**
 * A managed object maintains an internal thread-safe count of how many things are referencing it.
 * References are managed with Handles, which modify the internal reference count.
 * When the reference count of a managed object is 0, it is safe to destroy.
 */
struct ManagedObject {
public:
	/** A handle to a managed object. This handle will increase the reference count on the managed object. */
	template<typename ManagedObjectType>
	struct Handle {
		static_assert(std::is_base_of_v<ManagedObject, ManagedObjectType>, "Handle template parameter must derive from ManagedObject");
		template<typename A, typename B> friend Handle<A> Cast(Handle<B> const&);
		template<typename A, typename B> friend Handle<A> Cast(Handle<B>&&);

		/** Construct a new handle for a managed object. Unsafe to do unless the object is guaranteed not to be destroyed on another thread. */
		static Handle Create(ManagedObjectType& object) { return Handle{ object }; }

		Handle() = default;
		Handle(std::nullptr_t) noexcept : Handle() {}

		template<typename OtherManagedObjectType>
		Handle(Handle<OtherManagedObjectType> const& other) noexcept : object(other.object) {
			if (object) object->refCount.fetch_add(1);
		}
		template<typename OtherManagedObjectType>
		Handle(Handle<OtherManagedObjectType>&& other) noexcept : object(other.object) {
			other.object = nullptr;
		}

		~Handle() noexcept { if (object) object->refCount.fetch_sub(1); }

		template<typename OtherManagedObjectType>
		Handle& operator=(Handle<OtherManagedObjectType> const& other) { Handle(other).Swap(*this); }
		template<typename OtherManagedObjectType>
		Handle& operator=(Handle<OtherManagedObjectType>&& other) { Handle(std::move(other)).Swap(*this); }

		inline ManagedObjectType* operator->() const { return object; }
		inline ManagedObjectType& operator*() const { return *object; }
		inline operator bool() const { return !!object; }

		inline ManagedObjectType* Get() const { return object; }
		inline void Reset() { Handle().Swap(*this); }
		inline void Swap(Handle& other) { std::swap(object, other.object); }

	private:
		ManagedObjectType* object = nullptr;

		Handle(ManagedObjectType& inObject) noexcept : object(&inObject) {
			object->refCount.fetch_add(1);
		}
	};

	/** Get the current number of references to this object */
	size_t GetRefCount() const { return refCount.load(); }

private:
	std::atomic<size_t> refCount;
};

static_assert(std::is_trivially_destructible_v<ManagedObject>, "ManagedObject base class should be trivially destructible");

/** Create a copy of a handle cast to a different type */
template<typename OtherManagedObjectType, typename ManagedObjectType>
ManagedObject::Handle<OtherManagedObjectType> Cast(ManagedObject::Handle<ManagedObjectType> const& handle) {
	if (handle.object) return ManagedObject::Handle<OtherManagedObjectType>{ *static_cast<OtherManagedObjectType*>(handle.object) };
	else return nullptr;
}

/** Move a handle to a different type */
template<typename OtherManagedObjectType, typename ManagedObjectType>
ManagedObject::Handle<OtherManagedObjectType> Cast(ManagedObject::Handle<ManagedObjectType>&& handle) {
	ManagedObject::Handle<OtherManagedObjectType> other;
	other.object = static_cast<OtherManagedObjectType*>(handle.object);
	handle.object = nullptr;
	return other;
}
