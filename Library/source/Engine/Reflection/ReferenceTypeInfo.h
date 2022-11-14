#pragma once
#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Reflection/TypeInfo.h"

/*
std::shared_ptr<Type> can always cast to std::shared_ptr<void>, which will preserve the reference-counting behavior
std::shared_ptr<DerivedType> can be used to assign std::shared_ptr<Type> (referenced types allow raw assignment)
*/

namespace Reflection {
	/** A reference accessor instance which allows a reference object to be accessed while also keeping it referenced within the current scope */
	struct ReferenceAccessor {
		virtual void* GetMutable() const = 0;
		virtual void const* GetImmutable() const = 0;
	};
	using ScopedReferenceAccessor = std::unique_ptr<ReferenceAccessor const>;

	/** TypeInfo for a reference type. Reference types hold a reference to some externally-owned object. */
	struct ReferenceTypeInfo : public TypeInfo {
		/** The type of reference ownership used by the type */
		enum class EOwnershipType : uint8_t {
			Raw, //Object*
			SharedWeak, //std::shared_ptr<Object> and std::weak_ptr<Object>
			Managed, //ManagedObject::Handle<Object>
		};

		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Reference;

		/** The type of the referenced object. The actual reference may be a derived type. */
		TypeInfo const* baseType = nullptr;
		/** The ownership type used by this reference */
		EOwnershipType ownership = EOwnershipType::Raw;
		/** True if the referenced object is immutable when accessed through this reference */
		bool isImmutable = false;

		virtual ~ReferenceTypeInfo() = default;

		/** Get an accessor for the object an instance is referencing */
		virtual ScopedReferenceAccessor GetAccessor(void const* instance) const = 0;

		/** Copy an existing reference into an instance, so the instance is also referencing the same object */
		virtual bool Copy(void* instance, void const* other, ReferenceTypeInfo const* otherType) const = 0;
		/** Reset an instance so it no longer holds a reference */
		virtual void Reset(void* instance) = 0;

	protected:
		template<bool IsImmutable>
		bool CopyImpl(void* instance, void const* other, ReferenceTypeInfo const* otherType) const {
			if (ownership == otherType->ownership && (baseType == otherType->baseType || StructTypeInfo::IsDerivedStruct(this, otherType))) {
				if constexpr (IsImmutable) {
					CopyImmutable(instance, other, otherType);
					return true;
				} else {
					if (!otherType->isImmutable) {
						CopyMutable(instance, other, otherType);
						return true;
					}
					//Cannot copy an immutable reference into a mutable reference.
				}
			}
			return false;
		}
		virtual bool CopyImmutable(void* instance, void const* other, ReferenceTypeInfo const* otherType) const = 0;
		virtual bool CopyMutable(void* instance, void const* other, ReferenceTypeInfo const* otherType) const = 0;
	};

	//============================================================
	// Raw Ownership

	template<typename PointerType>
	struct RawReferenceAccessor {
		virtual void* GetMutable() const final { return reference; }
		virtual void const* GetImmutable() const final { return reference; }

		RawReferenceAccessor(PointerType inReference) : reference(inReference) {}

	private:
		PointerType reference;
	};

	struct RawPointerTypeInfo : public ReferenceTypeInfo {
		using ReferenceTypeInfo::ReferenceTypeInfo;

		~RawPointerTypeInfo() = default;

	protected:
		virtual void* GetErasedMutable(void const* instance) const = 0;
		virtual void const* GetErasedImmutable(void const* instance) const = 0;
	};

	template<typename PointerType, typename QualifiedBaseType>
	struct TRawPointerTypeInfo : public ImplementedTypeInfo<PointerType, RawPointerTypeInfo> {
		using BaseType = std::decay_t<QualifiedBaseType>;
		static constexpr bool IsImmutable = std::is_const_v<QualifiedBaseType>;

		static_assert(!std::is_void_v<BaseType>, "Reference of void is not supported for reflection. Internal value reflection is unsafe.");

		using ImplementedTypeInfo<PointerType, RawPointerTypeInfo>::Cast;
		using ReferenceTypeInfo::baseType;
		using ReferenceTypeInfo::ownership;
		using ReferenceTypeInfo::isImmutable;

		TSharedPointerTypeInfo(std::string_view inName) : ImplementedTypeInfo<PointerType, RawPointerTypeInfo>(Reflect<PointerType>::ID, inName) {
			baseType = Reflect<BaseType>::Get();
			ownership = EOwnershipType::SharedWeak;
			isImmutable = IsImmutable;
		}

		virtual ScopedReferenceAccessor GetAccessor(void const* instance) const {
			return std::make_unique<RawReferenceAccessor<PointerType>>(Cast(instance));
		}

		virtual bool Copy(void* instance, void const* other, ReferenceTypeInfo const* otherType) const final {
			CopyImpl<IsImmutable>(instance, other, otherType);
		}

	protected:
		virtual bool CopyImmutable(void* instance, void const* other, ReferenceTypeInfo const* otherType) const = 0;
		virtual bool CopyMutable(void* instance, void const* other, ReferenceTypeInfo const* otherType) const = 0;

		virtual void* GetErasedMutable(void const* instance) const final {
			if constexpr (IsImmutable) return nullptr;
			else return Cast(instance);
		}
		virtual void const* GetErasedImmutable(void const* instance) const final { return Cast(instance); }
	};

	//============================================================
	// Shared-Weak Ownership

	template<typename PointerType>
	struct SharedWeakReferenceAccessor {
		virtual void* GetMutable() const final { return reference.get(); }
		virtual void const* GetImmutable() const final { return reference.get(); }

		SharedWeakMutableReferenceAccessor(PointerType const& inReference) : reference(inReference) {}

	private:
		PointerType reference;
	};

	struct SharedWeakPointerTypeInfo : public ReferenceTypeInfo {
		using ReferenceTypeInfo::ReferenceTypeInfo;

		~SharedWeakPointerTypeInfo() = default;

		virtual std::shared_ptr<void> GetErasedMutable(void const* instance) const = 0;
		virtual std::shared_ptr<void const> GetErasedImmutable(void const* instance) const = 0;
	};

	template<typename PointerType, typename QualifiedBaseType>
	struct TSharedPointerTypeInfo : public ImplementedTypeInfo<PointerType, SharedWeakPointerTypeInfo> {
		using BaseType = std::decay_t<QualifiedBaseType>;
		static constexpr bool IsImmutable = std::is_const_v<QualifiedBaseType>;

		static_assert(!std::is_void_v<BaseType>, "Reference of void is not supported for reflection. Internal value reflection is unsafe.");

		using ImplementedTypeInfo<PointerType, SharedWeakPointerTypeInfo>::Cast;
		using ReferenceTypeInfo::baseType;
		using ReferenceTypeInfo::ownership;
		using ReferenceTypeInfo::isImmutable;

		TSharedPointerTypeInfo(std::string_view inName) : ImplementedTypeInfo<PointerType, SharedWeakPointerTypeInfo>(Reflect<PointerType>::ID, inName) {
			baseType = Reflect<BaseType>::Get();
			ownership = EOwnershipType::SharedWeak;
			isImmutable = IsImmutable;
		}

		virtual ScopedReferenceAccessor GetAccessor(void const* instance) const {
			return std::make_unique<SharedWeakReferenceAccessor<PointerType>>(Cast(instance));
		}

		virtual bool Copy(void* instance, ReferenceTypeInfo const* otherType, void const* other) const final {
			if (ownership == otherType->ownership && (baseType == otherType->baseType || IsDerivedStruct(this, otherType))) {
				if constexpr (IsImmutable) {
					Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(static_cast<SharedWeakPointerTypeInfo const*>(otherType)->GetErasedImmutable(other));
					return true;
				} else {
					if (!otherType->isImmutable) {
						Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(static_cast<SharedWeakPointerTypeInfo const*>(otherType)->GetErasedMutable(other));
						return true;
					}
					//Cannot copy an immutable reference into a mutable reference.
				}
			}
			return false;
		}

		virtual std::shared_ptr<void> GetErasedMutable(void const* instance) const final {
			if constexpr (IsImmutable) return nullptr;
			else return std::static_pointer_cast<void>(Cast(instance));
		}
		virtual std::shared_ptr<void const> GetErasedImmutable(void const* instance) const final {
			return std::static_pointer_cast<void const>(Cast(instance));
		}
	};

	template<typename PointerType, typename QualifiedBaseType>
	struct TWeakPointerTypeInfo : public ImplementedTypeInfo<PointerType, SharedWeakPointerTypeInfo> {
		using BaseType = std::decay_t<QualifiedBaseType>;
		static constexpr bool IsImmutable = std::is_const_v<QualifiedBaseType>;

		static_assert(!std::is_void_v<BaseType>, "Reference of void is not supported for reflection. Internal value reflection is unsafe.");

		using ImplementedTypeInfo<PointerType, SharedWeakPointerTypeInfo>::Cast;
		using ReferenceTypeInfo::baseType;
		using ReferenceTypeInfo::ownership;
		using ReferenceTypeInfo::isImmutable;

		TWeakPointerTypeInfo(std::string_view inName) : ImplementedTypeInfo<PointerType, SharedWeakPointerTypeInfo>(Reflect<PointerType>::ID, inName) {
			baseType = Reflect<BaseType>::Get();
			ownership = EOwnershipType::SharedWeak;
			isImmutable = IsImmutable;
		}

		virtual ScopedReferenceAccessor GetAccessor(void const* instance) const {
			return std::make_unique<SharedWeakReferenceAccessor<QualifiedBaseType>>(Cast(instance).lock());
		}

		virtual bool Copy(void* instance, ReferenceTypeInfo const* otherType, void const* other) const final {
			if (ownership == otherType->ownership && (baseType == otherType->baseType || IsDerivedStruct(this, otherType))) {
				if constexpr (IsImmutable) {
					Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(static_cast<SharedWeakPointerTypeInfo const*>(otherType)->GetErasedImmutable(other));
					return true;
				} else {
					if (!otherType->isImmutable) {
						Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(static_cast<SharedWeakPointerTypeInfo const*>(otherType)->GetErasedMutable(other));
						return true;
					}
					//Cannot copy an immutable reference into a mutable reference.
				}
			}
			return false;
		}

		virtual std::shared_ptr<void> GetErasedMutable(void const* instance) const final {
			if constexpr (IsImmutable) return nullptr;
			else return std::static_pointer_cast<void>(Cast(instance).lock());
		}
		virtual std::shared_ptr<void const> GetErasedImmutable(void const* instance) const final {
			return std::static_pointer_cast<void const>(Cast(instance).lock());
		}
	};
}
