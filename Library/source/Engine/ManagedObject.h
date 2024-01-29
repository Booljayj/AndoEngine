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
		HandleBase(HandleBase const& other) : HandleBase(other.object) {}
		HandleBase(HandleBase&& other) noexcept : object(other.object) {
			other.object = nullptr;
		}
		HandleBase(ManagedObject* inObject) : object(inObject) {
			if (object) object->refCount.fetch_add(1);
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
	template<typename ObjectType_>
	struct Handle {
		using ObjectType = ObjectType_;
		using MutableObjectType = std::remove_const_t<ObjectType>;

		template<typename A> friend struct Handle;
		template<typename A, typename B> friend Handle<A> Cast(Handle<B> const&);
		template<typename A, typename B> friend Handle<A> Cast(Handle<B>&&);

		/** A factory which is allowed to create handles from a raw object reference. It is assumed to be the instance managing the raw object references. */
		struct Factory {
		protected:
			/** Create a new handle from a raw object reference. Must only be called in a thread-safe context. */
			static Handle<ObjectType> CreateHandle(ObjectType& object, ManagedObject& control) {
				return Handle<ObjectType>{ object, control };
			}
		};

		Handle() noexcept = default;
		Handle(std::nullptr_t) noexcept : Handle() {}
		~Handle() { if (control) control->refCount.fetch_sub(1); }

		Handle(Handle const& other) noexcept : Handle(other.object, other.control) {}
		Handle(Handle&& other) noexcept { *this = std::move(other); }

		Handle& operator=(Handle const& other) { Handle(other).Swap(*this); return *this; }
		Handle& operator=(Handle&& other) {
			std::swap(object, other.object);
			std::swap(control, other.control);
			return *this;
		}

		template<std::derived_from<MutableObjectType> OtherObjectType>
		Handle(Handle<OtherObjectType> const& other) noexcept : Handle(static_cast<MutableObjectType*>(other.object), other.control) {
			static_assert(std::is_base_of_v<ManagedObject, ObjectType>, "Handle template parameter must derive from ManagedObject");
			static_assert(std::is_base_of_v<ObjectType, OtherObjectType>, "Cannot implicitly cast handle to target type");
		}
		template<std::derived_from<MutableObjectType> OtherObjectType>
		Handle(Handle<OtherObjectType>&& other) noexcept {
			static_assert(std::is_base_of_v<ManagedObject, ObjectType>, "Handle template parameter must derive from ManagedObject");
			static_assert(std::is_base_of_v<ObjectType, OtherObjectType>, "Cannot implicitly cast handle to target type");
			object = static_cast<MutableObjectType*>(other.object);
			control = other.control;
			other.object = nullptr;
			other.control = nullptr;
		}

		template<typename OtherObjectType>
		Handle& operator=(Handle<OtherObjectType> const& other) {
			static_assert(std::is_base_of_v<ManagedObject, ObjectType>, "Handle template parameter must derive from ManagedObject");
			static_assert(std::is_base_of_v<ObjectType, OtherObjectType>, "Cannot implicitly cast handle to target type");
			Handle(static_cast<MutableObjectType*>(other.object), other.control).Swap(*this);
			return *this;
		}
		template<typename OtherObjectType>
		Handle& operator=(Handle<OtherObjectType>&& other) {
			static_assert(std::is_base_of_v<ManagedObject, ObjectType>, "Handle template parameter must derive from ManagedObject");
			static_assert(std::is_base_of_v<ObjectType, OtherObjectType>, "Cannot implicitly cast handle to target type");
			*this = Handle(static_cast<MutableObjectType*>(other.object), other.control);
			return *this;
		}

		inline operator bool() const { return !!control; }

		inline ObjectType* operator->() const { return static_cast<ObjectType*>(object); }
		inline ObjectType& operator*() const { return *static_cast<ObjectType*>(object); }

		inline ObjectType* Get() const { return static_cast<ObjectType*>(object); }
		inline void Reset() { Handle().Swap(*this); }
		inline void Swap(Handle& other) { std::swap(object, other.object); std::swap(control, other.control); }

	private:
		MutableObjectType* object = nullptr;
		ManagedObject* control = nullptr;

		Handle(MutableObjectType* inObject, ManagedObject* inControl) noexcept : object(inObject), control(inControl) {
			if (control) control->refCount.fetch_add(1);
		}

		Handle(MutableObjectType& inObject, ManagedObject& inControl) noexcept : object(&inObject), control(&inControl) {
			control->refCount.fetch_add(1);
		}
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
	static_assert(std::is_base_of_v<OtherObjectType, ObjectType> || std::is_base_of_v<ObjectType, OtherObjectType>, "Cannot cast handles between unrelated types");
	using MutableOtherObjectType = ManagedObject::Handle<OtherObjectType>::MutableObjectType;

	if (handle.control) return ManagedObject::Handle<MutableOtherObjectType>{ *static_cast<MutableOtherObjectType*>(handle.object), *handle.control };
	else return nullptr;
}

/** Convert a handle to a different type */
template<typename OtherObjectType, typename ObjectType>
ManagedObject::Handle<OtherObjectType> Cast(ManagedObject::Handle<ObjectType>&& handle) {
	static_assert(std::is_base_of_v<OtherObjectType, ObjectType> || std::is_base_of_v<ObjectType, OtherObjectType>, "Cannot cast handles between unrelated types");
	using MutableOtherObjectType = ManagedObject::Handle<OtherObjectType>::MutableObjectType;

	ManagedObject::Handle<OtherObjectType> newHandle;
	newHandle.object = static_cast<MutableOtherObjectType*>(handle.object);
	newHandle.control = handle.control;
	handle.object = nullptr;
	handle.control = nullptr;
	return newHandle;
}


//TEST
namespace
{
	struct Base : ManagedObject {};
	struct Child : Base {};
	struct Unrelated : ManagedObject {};
	struct NotManaged {};

	void Test() {
		ManagedObject::Handle<Base> base;
		ManagedObject::Handle<Child> child;
		ManagedObject::Handle<Unrelated> urelated;
		ManagedObject::Handle<NotManaged> notManaged;

		//These should succeed
		child = Cast<Child>(base);
		base = Cast<Base>(child);
		base = child;

		//These should fail
		child = base;
		urelated = base;
		notManaged = Cast<NotManaged>(base);
	}
}
