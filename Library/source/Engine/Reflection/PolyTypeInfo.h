#pragma once
#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Reflection/TypeInfo.h"
#include "Engine/Reflection/TypeInfoReference.h"

namespace Reflection {
	/** TypeInfo for a poly type. "Poly" is short for Polymorphic Interface. A poly owns a pointer to an optional instance that can be cast to a base type. */
	struct PolyTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Poly;

		/** The base type for this poly */
		StructTypeInfo const* base = nullptr;

		virtual ~PolyTypeInfo() = default;

		inline bool CanAssignType(StructTypeInfo const& type) const {
			if (type.flags.Has(ETypeFlags::Abstract)) return false;
			else if (type.IsChildOf(*base)) return true;
			else return false;
		}

		/** Get the current value of the poly. Can be nullptr if the poly is unassigned */
		virtual void* GetValue(void* instance) const = 0;
		virtual void const* GetValue(void const* instance) const = 0;

		/** Get the current type of the poly. Can be nullptr if the poly is unassigned, otherwise can be anything deriving from base. */
		virtual StructTypeInfo const* GetType(void const* instance) const = 0;

		/** Assign a new value to a poly */
		virtual bool Assign(void* instance, TypeInfo const& type, void const* value) const = 0;
		/** Unassign the value of a poly */
		virtual void Unassign(void* instance) const = 0;
	};

	//============================================================
	// Templates

	template<typename PointerType, Concepts::ReflectedStruct BaseType>
	struct TPointerTypeInfo : public ImplementedTypeInfo<PointerType, PolyTypeInfo> {
		using ImplementedTypeInfo<PointerType, PolyTypeInfo>::Cast;
		using PolyTypeInfo::base;

		TPointerTypeInfo(std::string_view name) : ImplementedTypeInfo<PointerType, PolyTypeInfo>(::Reflection::Reflect<PointerType>::ID, name) {
			base = ::Reflection::Reflect<BaseType>::Get();
		}

		virtual void* GetValue(void* instance) const final { return Cast(instance).get(); }
		virtual void const* GetValue(void const* instance) const final { return Cast(instance).get(); }

		virtual bool Assign(void* instance, TypeInfo const& type, void const* value) const final {
			if (PolyTypeInfo::CanAssignType(type)) {
				auto newInstance = type.Allocate();
				if (value) type.Construct(newInstance.get(), value);
				else type.Construct(newInstance.get());
				Cast(instance).reset(reinterpret_cast<BaseType*>(newInstance.release()));
				return true;
			} else {
				return false;
			}
		}
		virtual void Unassign(void* instance) const final { Cast(instance).reset(); }

		TYPEINFO_BUILDER_METHODS(PointerType)
	};

	//============================================================
	// Standard poly reflection

	template<typename BaseType>
	struct Reflect<std::unique_ptr<BaseType>> {
		static PolyTypeInfo const& Get() { return info; }
		static constexpr Hash128 ID = Hash128{ "std::unique_ptr"sv } + Reflect<BaseType>::ID;
	private:
		using ThisTypeInfo = TPointerTypeInfo<std::unique_ptr<BaseType>, BaseType>;
		static ThisTypeInfo const info;
	};
	template<typename BaseType>
	typename Reflect<std::unique_ptr<BaseType>>::ThisTypeInfo const Reflect<std::unique_ptr<BaseType>>::info =
		Reflect<std::unique_ptr<BaseType>>{ "std::unique_ptr"sv }
		.Description("unique pointer"sv);
}

namespace YAML {
	template<typename T>
	struct convert<std::unique_ptr<T>> {
		static Node encode(const std::unique_ptr<T>& instance) {
			Reflection::PolyTypeInfo const& type = Reflect<std::unique_ptr<T>>::Get();
			
			Node node{ NodeType::Map };

			if (Reflection::StructTypeInfo const* actual = type.GetType(&instance)) {
				node["type"] = Reflection::TypeInfoReference{ *actual };

				if (Node const value = actual->Serialize(type.GetValue(&instance))) {
					node["value"] = value;
				}

			} else {
				node["type"] = Reflection::TypeInfoReference{};
			}

			return node;
		}

		static bool decode(const Node& node, std::unique_ptr<T>& instance) {
			Reflection::PolyTypeInfo const& type = Reflect<std::unique_ptr<T>>::Get();

			if (!node.IsMap()) return false;

			Reflection::TypeInfoReference const reference = node["type"].as<Reflection::TypeInfoReference>();
			if (Reflection::StructTypeInfo const* actual = reference.Resolve<Reflection::StructTypeInfo>()) {
				if (!type.Assign(&instance, *actual, nullptr)) return false;

				if (Node const value = node["value"]) actual->Deserialize(value, type.GetValue(&instance));

			}
			else {
				type.Unassign(&instance);
			}
		}
	};
}
