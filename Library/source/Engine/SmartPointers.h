#pragma once
#include <memory>
#include "Engine/Core.h"
#include "Engine/Reflection/TypeInfo.h"
#include "Engine/Reflection/TypeInfoReference.h"

//Shared and unique pointers are not serializable by default, because there's no generic way to handle loading for all types.
//However, serialization may be enabled for some subtypes, and those types will also provide a way to retrieve the r

namespace Archive {
	template<typename T>
	struct Serializer<std::shared_ptr<T>> {
		static void Write(Output& archive, std::shared_ptr<T> const& reference) {}
		static void Read(Input& archive, std::shared_ptr<T>& reference) { reference.reset(); }
	};

	template<Reflection::Concepts::ReflectedStructType T>
	struct Serializer<std::unique_ptr<T>> {
		static void Write(Output& archive, std::unique_ptr<T> const& instance) {
			if (instance) {
				archive << true;
				Reflect<T>::Get().Serialize(archive, instance.get());
			} else {
				archive << false;
			}
		}
		static void Read(Input& archive, std::unique_ptr<T>& instance) {
			bool valid = false;
			archive >> valid;
			if (valid) {
				instance = Reflect<T>::Get().AllocateInstance();
				Reflect<T>::Get().Deserialize(archive, instance.get());
			} else {
				instance.reset();
			}
		}
	};

	template<typename T>
	struct Serializer<std::unique_ptr<T>> {
		static void Write(Output& archive, std::unique_ptr<T> const& instance) {
			if constexpr (Reflection::Concepts::HasGetTypeInfoMethod<T>) {
				if (instance) {
					Reflection::StructTypeInfo const& type = instance->GetTypeInfo();
					Serializer<Reflection::TypeInfoReference>::Write(archive, Reflection::TypeInfoReference{ type });
					type.Serialize(archive, instance.get());
				} else {
					Serializer<Reflection::TypeInfoReference>::Write(archive, Reflection::TypeInfoReference{});
				}
			} else {
				Reflection::StructTypeInfo const& type = Reflect<T>::Get();
				if (instance) {
					archive << true;
					type.Serialize(archive, instance.get());
				} else {
					archive << false;
				}
			}
		}
		static void Read(Input& archive, std::unique_ptr<T>& instance) {
			if constexpr (Reflection::Concepts::HasGetTypeInfoMethod<T>) {
				Reflection::TypeInfoReference type_reference;
				Serializer<Reflection::TypeInfoReference>::Read(archive, type_reference);
				if (Reflection::StructTypeInfo const* type = type_reference.Resolve()) {
					instance = type->Allocate<T>();
					type->Deserialize(archive, instance.get());
				} else {

				}
			} else {
				bool valid = false;
				archive >> valid;
				if (valid) {

				} else {

				}
			}
		}
	};
}

namespace YAML {
	template<Reflection::Concepts::HasGetTypeInfoMethod T>
	struct convert<std::unique_ptr<T>> {
		static Node encode(const std::unique_ptr<T>& instance) {
			Node node{ NodeType::Map };

			if (instance) {
				if constexpr (Reflection::Concepts::HasGetTypeInfoMethod<T>) {
					Reflection::StructTypeInfo const& type = instance->GetTypeInfo();
					node["type"] = Reflection::TypeInfoReference{ type };
					node["value"] = type.Serialize(instance.get());
				} else {
					Reflection::StructTypeInfo const& type = Reflect<T>::Get();
					node["type"] = Reflection::TypeInfoReference{ type };
					node["value"] = type.Serialize(instance.get());
				}
			} else {
				node["type"] = Reflection::TypeInfoReference{};
			}

			return node;
		}

		static bool decode(const Node& node, std::unique_ptr<T>& instance) {
			if (!node.IsMap()) return false;

			Reflection::TypeInfoReference const reference = node["type"].as<Reflection::TypeInfoReference>();
			if (Reflection::StructTypeInfo const* type = reference.Resolve<Reflection::StructTypeInfo>()) {
				instance = type->Allocate<T>();
				if (Node const value = node["value"]) type->Deserialize(value, instance.get());
			} else {
				instance.reset();
			}
		}
	};
}

namespace Reflection {
	template<typename PointerType, Concepts::ReflectedStructType QualifiedBaseType>
	struct TSharedPointerTypeInfo : public ImplementedTypeInfo<PointerType, ReferenceTypeInfo> {
		using BaseType = std::decay_t<QualifiedBaseType>;

		using ImplementedTypeInfo<PointerType, ReferenceTypeInfo>::Cast;
		using ReferenceTypeInfo::base;
		using ReferenceTypeInfo::isImmutable;

		TSharedPointerTypeInfo(std::u16string_view name, std::u16string_view description) : ImplementedTypeInfo<PointerType, ReferenceTypeInfo>(Reflect<PointerType>::ID, name, description) {
			base = &Reflect<BaseType>::Get();
			isImmutable = std::is_const_v<QualifiedBaseType>;
		}

		virtual std::shared_ptr<void> GetMutable(void const* instance) const final {
			if constexpr (std::is_const_v<QualifiedBaseType>) return nullptr;
			else return std::static_pointer_cast<void>(Cast(instance));
		}
		virtual std::shared_ptr<void const> GetImmutable(void const* instance) const final {
			return std::static_pointer_cast<void const>(Cast(instance));
		}

		virtual void Reset(void* instance) override final { Cast(instance).reset(); }
	};

	template<typename PointerType, Concepts::ReflectedStructType QualifiedBaseType>
	struct TWeakPointerTypeInfo : public ImplementedTypeInfo<PointerType, ReferenceTypeInfo> {
		using BaseType = std::decay_t<QualifiedBaseType>;

		using ImplementedTypeInfo<PointerType, ReferenceTypeInfo>::Cast;
		using ReferenceTypeInfo::base;
		using ReferenceTypeInfo::isImmutable;

		TWeakPointerTypeInfo(std::u16string_view name, std::u16string_view description) : ImplementedTypeInfo<PointerType, ReferenceTypeInfo>(Reflect<PointerType>::ID, name, description) {
			base = &Reflect<BaseType>::Get();
			isImmutable = std::is_const_v<QualifiedBaseType>;
		}

		virtual std::shared_ptr<void> GetMutable(void const* instance) const final {
			if constexpr (std::is_const_v<QualifiedBaseType>) return nullptr;
			else return std::static_pointer_cast<void>(Cast(instance).lock());
		}
		virtual std::shared_ptr<void const> GetImmutable(void const* instance) const final {
			return std::static_pointer_cast<void const>(Cast(instance).lock());
		}

		virtual void Reset(void* instance) override final { Cast(instance).reset(); }
	};

	template<typename PointerType, Concepts::ReflectedStructType BaseType>
	struct TUniquePointerTypeInfo : public ImplementedTypeInfo<PointerType, PolyTypeInfo> {
		using ImplementedTypeInfo<PointerType, PolyTypeInfo>::Cast;
		using PolyTypeInfo::base;

		TUniquePointerTypeInfo(std::u16string_view name, std::u16string_view description)
			: ImplementedTypeInfo<PointerType, PolyTypeInfo>(::Reflect<PointerType>::ID, name, description)
			, base(::Reflect<BaseType>::Get())
		{
		}

		virtual void* GetValue(void* instance) const final { return Cast(instance).get(); }
		virtual void const* GetValue(void const* instance) const final { return Cast(instance).get(); }

		virtual bool Assign(void* instance, StructTypeInfo const& type, void const* value) const final {
			if (PolyTypeInfo::CanAssignType(type)) {
				auto newInstance = type.Allocate<BaseType>();
				if (value) type.Construct(newInstance.get(), value);
				else type.Construct(newInstance.get());
				Cast(instance) = std::move(newInstance);
				return true;
			}
			else {
				return false;
			}
		}
		virtual void Unassign(void* instance) const final { Cast(instance).reset(); }
	};
}

template<Reflection::Concepts::ReflectedStructType Type>
struct Reflect<std::shared_ptr<Type>> {
	static ::Reflection::ReferenceTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::shared_ptr"sv } + Reflect<Type>::ID;
private:
	using ThisTypeInfo = ::Reflection::TSharedPointerTypeInfo<std::shared_ptr<Type>, Type>;
	static ThisTypeInfo const info;
};

template<Reflection::Concepts::ReflectedStructType Type>
Reflect<std::shared_ptr<Type>>::ThisTypeInfo const Reflect<std::shared_ptr<Type>>::info{ u"std::shared_ptr"sv, u"shared pointer"sv };

template<Reflection::Concepts::ReflectedStructType Type>
struct Reflect<std::weak_ptr<Type>> {
	static ::Reflection::ReferenceTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::weak_ptr"sv } + Reflect<Type>::ID;
private:
	using ThisTypeInfo = ::Reflection::TWeakPointerTypeInfo<std::shared_ptr<Type>, Type>;
	static ThisTypeInfo const info;
};

template<Reflection::Concepts::ReflectedStructType Type>
Reflect<std::weak_ptr<Type>>::ThisTypeInfo const Reflect<std::weak_ptr<Type>>::info{ u"std::weak_ptr"sv, u"weak pointer"sv };

template<typename BaseType>
struct Reflect<std::unique_ptr<BaseType>> {
	static ::Reflection::PolyTypeInfo const& Get() { return info; }
	static constexpr Hash128 ID = Hash128{ "std::unique_ptr"sv } + Reflect<BaseType>::ID;
private:
	using ThisTypeInfo = ::Reflection::TUniquePointerTypeInfo<std::unique_ptr<BaseType>, BaseType>;
	static ThisTypeInfo const info;
};
template<typename BaseType>
typename Reflect<std::unique_ptr<BaseType>>::ThisTypeInfo const Reflect<std::unique_ptr<BaseType>>::info{ u"std::unique_ptr"sv, u"unique pointer"sv };
