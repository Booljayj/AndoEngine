#pragma once
#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	/**
	 * TypeInfo for a reference type. Reference types hold a reference to some externally-owned object.
	 * A reference is always a shared_ptr or weak_ptr, which implement the referencing mechanics.
	 */
	struct ReferenceTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Reference;

		/** The type of the referenced object. The actual reference may be a derived type. */
		TypeInfo const* base = nullptr;
		/** True if the referenced object is immutable when accessed through this reference */
		bool isImmutable = false;

		virtual ~ReferenceTypeInfo() = default;

		/** Get a type-erased version of this reference to a mutable object */
		virtual std::shared_ptr<void> GetMutable(void const* instance) const = 0;
		/** Get a type-erased version of this reference to an immutable object */
		virtual std::shared_ptr<void const> GetImmutable(void const* instance) const = 0;

		/** Copy an existing reference into an instance, so the instance is also referencing the same object */
		virtual bool Copy(void* instance, void const* other, ReferenceTypeInfo const* otherType) const = 0;
		/** Reset an instance so it no longer holds a reference */
		virtual void Reset(void* instance) = 0;

		/** Get a version of this reference as the provided type. Does not check if the type conversion is valid, that should be done before calling */
		template<typename T>
		std::shared_ptr<T> GetMutable(void const* instance) const { return std::static_pointer_cast<T>(GetMutable(instance)); }
		/** Get a version of this reference as the provided type. Does not check if the type conversion is valid, that should be done before calling */
		template<typename T>
		std::shared_ptr<T const> GetImmutable(void const* instance) const { return std::static_pointer_cast<T const>(GetImmutable(instance)); }

		/** Returns true if the type contained in the reference is a chid of the provided parent type (or equal) */
		inline bool IsReferenceChildOf(StructTypeInfo const& parent) const {
			auto const* struct_base = Cast<StructTypeInfo>(base);
			if (struct_base && struct_base->IsChildOf(parent)) return true;
			else return false;
		}

		/** Returns true if the type contained in the reference is a chid of the provided parent type (or equal) */
		template<Concepts::ReflectedStruct T>
		inline bool IsReferenceChildOf() const {
			auto const* struct_base = Cast<StructTypeInfo>(base);
			if (struct_base && struct_base->IsChildOf<T>()) return true;
			else return false;
		}

	protected:
		template<bool IsImmutable>
		bool CopyImpl(void* instance, void const* other, ReferenceTypeInfo const* otherType) const {
			if (base == otherType->base) {
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

	template<typename PointerType, typename QualifiedBaseType>
	struct TSharedPointerTypeInfo : public ImplementedTypeInfo<PointerType, ReferenceTypeInfo> {
		using BaseType = std::decay_t<QualifiedBaseType>;
		static constexpr bool IsImmutable = std::is_const_v<QualifiedBaseType>;

		static_assert(!std::is_void_v<BaseType>, "Reference of void is not supported for reflection. Internal value reflection is unsafe.");

		using ImplementedTypeInfo<PointerType, ReferenceTypeInfo>::Cast;
		using ReferenceTypeInfo::base;
		using ReferenceTypeInfo::isImmutable;

		TSharedPointerTypeInfo(std::string_view name) : ImplementedTypeInfo<PointerType, ReferenceTypeInfo>(Reflect<PointerType>::ID, name) {
			base = Reflect<BaseType>::Get();
			isImmutable = IsImmutable;
		}

		virtual std::shared_ptr<void> GetMutable(void const* instance) const final {
			if constexpr (IsImmutable) return nullptr;
			else return std::static_pointer_cast<void>(Cast(instance));
		}
		virtual std::shared_ptr<void const> GetImmutable(void const* instance) const final {
			return std::static_pointer_cast<void const>(Cast(instance));
		}

		virtual bool Copy(void* instance, ReferenceTypeInfo const* otherType, void const* other) const final {
			if (base == otherType->base || IsDerivedStruct(this, otherType)) {
				if constexpr (IsImmutable) {
					Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(otherType->GetImmutable(other));
					return true;
				} else {
					if (!otherType->isImmutable) {
						Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(otherType->GetMutable(other));
						return true;
					}
					//Cannot copy an immutable reference into a mutable reference.
				}
			}
			return false;
		}
		virtual void Reset(void* instance) { Cast(instance).reset(); }
	};

	template<typename PointerType, typename QualifiedBaseType>
	struct TWeakPointerTypeInfo : public ImplementedTypeInfo<PointerType, ReferenceTypeInfo> {
		using BaseType = std::decay_t<QualifiedBaseType>;
		static constexpr bool IsImmutable = std::is_const_v<QualifiedBaseType>;

		static_assert(!std::is_void_v<BaseType>, "Reference of void is not supported for reflection. Internal value reflection is unsafe.");

		using ImplementedTypeInfo<PointerType, ReferenceTypeInfo>::Cast;
		using ReferenceTypeInfo::base;
		using ReferenceTypeInfo::isImmutable;

		TWeakPointerTypeInfo(std::string_view inName) : ImplementedTypeInfo<PointerType, ReferenceTypeInfo>(Reflect<PointerType>::ID, inName) {
			base = Reflect<BaseType>::Get();
			isImmutable = IsImmutable;
		}

		virtual std::shared_ptr<void> GetMutable(void const* instance) const final {
			if constexpr (IsImmutable) return nullptr;
			else return std::static_pointer_cast<void>(Cast(instance).lock());
		}
		virtual std::shared_ptr<void const> GetImmutable(void const* instance) const final {
			return std::static_pointer_cast<void const>(Cast(instance).lock());
		}

		virtual bool Copy(void* instance, ReferenceTypeInfo const* otherType, void const* other) const final {
			if (base == otherType->base || IsDerivedStruct(this, otherType)) {
				if constexpr (IsImmutable) {
					Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(otherType->GetImmutable(other));
					return true;
				} else {
					if (!otherType->isImmutable) {
						Cast(instance) = std::static_pointer_cast<QualifiedBaseType>(otherType->GetMutable(other));
						return true;
					}
					//Cannot copy an immutable reference into a mutable reference.
				}
			}
			return false;
		}
		virtual void Reset(void* instance) { Cast(instance).reset(); }
	};
}
