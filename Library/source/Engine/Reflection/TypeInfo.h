#pragma once
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Engine/StandardTypes.h"
#include "Engine/TypeTraits.h"

#ifndef TARGET_NAME
#define TARGET_NAME Unknown
#endif

namespace Reflection {
	/** Classifications for types. Each classification contains types that share a similar usage or format. */
	enum class ETypeClassification : uint8_t {
		Unknown,

		//Basic construct types
		Primitive,
		Struct,
		Alias,
		Enumeration,
		Flags,
		//Homogeneous collection types
		Array,
		Map,
		Set,
		//Heterogeneous collection types
		Poly,
		Reference,
		Tuple,
		Variant,
	};
	
	/** flags that describe aspects of a particular type */
	enum class ETypeFlags : uint16_t {
		/** This type can be constructed with a default constructor */
		DefaultConstructable,
		/** This type can be copy-constructed from another instance */
		CopyConstructable,
		/** This type can be copy-assigned from another instance */
		CopyAssignable,
		/** This type can be compared for equality with another instance */
		EqualityComparable,

		/** This type is trivial */
		Trivial,
		/** This type is abstract */
		Abstract,
		/** This type is an implementation of a template */
		Template,

		/** This type can be serialized to disk (binary or text) */
		Serializable,
		/** This type can be serialized over the network (fast binary) */
		NetSerializable,
	};

	/** flags that describe aspects of a particular type */
	struct FTypeFlags : public TFlags<ETypeFlags> {
		TFLAGS_METHODS(FTypeFlags);

		/** Create a standard set of flags based on the type, plus the base flags */
		template<typename Type>
		constexpr static FTypeFlags Create() {
			FTypeFlags flags;
			if constexpr (std::default_initializable<Type>) flags += ETypeFlags::DefaultConstructable;
			if constexpr (std::copy_constructible<Type>) flags += ETypeFlags::CopyConstructable;
			if constexpr (std::copyable<Type>) flags += ETypeFlags::CopyAssignable;
			if constexpr (std::equality_comparable<Type>) flags += ETypeFlags::EqualityComparable;
			if constexpr (std::is_trivial_v<Type>) flags += ETypeFlags::Trivial;
			if constexpr (std::is_abstract_v<Type>) flags += ETypeFlags::Abstract;
			return flags;
		}
	};

	/** The memory parameters for a type */
	struct MemoryParams {
		/** The size of the type in bytes */
		size_t size = 0;
		/** The alignment of the type in bytes */
		size_t alignment = 0;

		template<typename Type>
		constexpr static MemoryParams Create() {
			if constexpr (!std::is_void_v<Type>) return MemoryParams{ sizeof(Type), alignof(Type) };
			else return MemoryParams{ 0, 0 };
		};
	};

	/** Deleter which deletes the memory allocated for an arbitrary type instance */
	struct TypeUniquePointerDeleter {
		void operator()(void* pointer);
	};

	/** A pointer to an arbitrary type instance created through the reflection system */
	using TypeUniquePointer = std::unique_ptr<std::byte[], TypeUniquePointerDeleter>;

	/** Provides a set of runtime information about a type */
	struct TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Unknown;

		/** The classification of this TypeInfo, defining what kinds of type information it contains */
		ETypeClassification classification = ETypeClassification::Primitive;
		/** Flags that provide additional information about this type */
		FTypeFlags flags;

		/** The identifier for this type. Always unique and stable. */
		Hash128 id;
		/** The fully-qualified name of this type. If this is a template, does not include template parameters. */
		std::string_view name;
		/** The memory parameters for this type */
		MemoryParams memory;

		/** Human-readable description of this type */
		std::string_view description;

		TypeInfo() = delete;
		virtual ~TypeInfo() = default;

		/** Allocate uninitialized memory large enough to hold an instance of this type, and return a unique pointer to that memory */
		TypeUniquePointer Allocate() const;

		/** Return the fully-qualified name of this type, including template parameters */
		virtual std::string GetName() const { return std::string{ name }; }

		/** Destruct an instance of this type at the address. Assumes the instance was properly constructed and won't be destructed again. */
		virtual void Destruct(void* instance) const = 0;
		/** Construct an instance of this type at the address using the default constructor */
		virtual void Construct(void* instance) const = 0;
		/** Construct an instance of this type at the address using the copy constructor and an existing instance */
		virtual void Construct(void* instance, void const* other) const = 0;
		/** Make the instance into a copy of the other instance. The instances should already be constructed. */
		virtual void Copy(void* instance, void const* other) const = 0;
		/** Compare two instances of this type and return true if they should be considered equal */
		virtual bool Equal(void const* a, void const* b) const = 0;

	protected:
		inline TypeInfo(ETypeClassification classification, FTypeFlags flags, Hash128 id, std::string_view name, MemoryParams memory)
			: classification(classification), flags(flags), id(id), name(name), memory(memory)
		{}
	};

	/** Struct which provides reflection information for a type. Must be specialized for all reflected types. Non-reflected types fail to compile if this is used. */
	template<typename Type>
	struct Reflect {
		//static ::Reflection::TypeInfo const& Get() { return Type::info; }
		//static constexpr Hash128 ID = Hash128{};
	};

	namespace Concepts {
		/** A struct deriving from TypeInfo, implementing behavior and data for a specific classification of types */
		template<typename T>
		concept DerivedFromTypeInfo = std::derived_from<T, TypeInfo> and requires {
			{ T::Classification } -> std::convertible_to<ETypeClassification>;
		};

		/** A type which implements reflection via the Reflect struct template */
		template<typename T>
		concept ReflectedType = requires (T a) {
			{ ::Reflection::Reflect<T>::Get() } -> std::convertible_to<TypeInfo const&>;
			{ ::Reflection::Reflect<T>::ID } -> std::convertible_to<Hash128>;
		};
	}

	/** Registers a TypeInfo instance so it can be found statically. Used for concrete known types, or types that need to be dynamically instanced. */
	struct RegisteredTypeInfo {
		static std::deque<TypeInfo const*> const& GetInfos() { return infos; }
		
		RegisteredTypeInfo(TypeInfo const& info);
		RegisteredTypeInfo(RegisteredTypeInfo const&) = delete;
		RegisteredTypeInfo(RegisteredTypeInfo&&) = delete;
		~RegisteredTypeInfo();

	private:
		static std::deque<TypeInfo const*> infos;
		TypeInfo const* cached;
	};

	/** Implements type-specific operation overrides for a particular TypeInfo instance. */
	template<std::destructible Type, Concepts::DerivedFromTypeInfo BaseType>
	struct ImplementedTypeInfo : public BaseType {
		static constexpr Type const& Cast(void const* pointer) { return *static_cast<Type const*>(pointer); }
		static constexpr Type& Cast(void* pointer) { return *static_cast<Type*>(pointer); }

		ImplementedTypeInfo(Hash128 id, std::string_view name)
			: BaseType(BaseType::Classification, FTypeFlags::Create<Type>(), id, name, MemoryParams::Create<Type>())
		{}

		virtual void Destruct(void* instance) const final {
			if constexpr (!std::is_trivially_destructible_v<Type>) Cast(instance).~Type();
		}
		virtual void Construct(void* instance) const final {
			if constexpr (std::is_default_constructible_v<Type>) new (instance) Type();
		}
		virtual void Construct(void* instance, void const* other) const final {
			if constexpr (std::is_copy_constructible_v<Type>) new (instance) Type(Cast(other));
			else Construct(instance);
		}
		virtual void Copy(void* instance, void const* other) const final {
			if constexpr (std::is_copy_assignable_v<Type>) Cast(instance) = Cast(other);
		}
		virtual bool Equal(void const* a, void const* b) const final {
			if constexpr (std::equality_comparable<Type>) return Cast(a) == Cast(b);
			else return false;
		}
	};
}

//============================================================
// Utility methods and aliases added to the global namespace for convenience

inline bool operator==(::Reflection::TypeInfo const& a, ::Reflection::TypeInfo const& b) { return a.id == b.id; }

template<typename T>
using Reflect = ::Reflection::Reflect<T>;

/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
template<Reflection::Concepts::DerivedFromTypeInfo TargetType, Reflection::Concepts::DerivedFromTypeInfo OriginalType>
TargetType const* Cast(OriginalType const* info) {
	//Upcasting or the same type does not need any special logic, so we can return the type directly
	if constexpr (std::derived_from<OriginalType, TargetType>) return static_cast<TargetType const*>(info);
	//Otherwise, check if the type's classification value matches the classification of TargetType
	if (info && info->classification == TargetType::Classification) return static_cast<TargetType const*>(info);
	else return nullptr;
}

/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
template<Reflection::Concepts::DerivedFromTypeInfo TargetType, Reflection::Concepts::DerivedFromTypeInfo OriginalType>
TargetType const* Cast(OriginalType const& info) {
	//Upcasting or the same type does not need any special logic, so we can return the type directly
	if constexpr (std::derived_from<OriginalType, TargetType>) return static_cast<TargetType const*>(&info);
	//Otherwise, check if the type's classification value matches the classification of TargetType
	if (info.classification == TargetType::Classification) return static_cast<TargetType const*>(&info);
	else return nullptr;
}

/** Define a Reflect implementation that only resolves if the type uses {Name}TypeInfo. Equal in behavior to calling Cast<{Name}TypeInfo>(Reflect<Type>::Get()). */
#define TYPEINFO_REFLECT(Name)\
template<typename Type>\
struct Reflect ## Name {\
	static ::Reflection::Name ## TypeInfo const& Get() {\
		if constexpr (std::is_base_of_v<::Reflection::Name ## TypeInfo, std::remove_pointer_t<decltype(Reflect<Type>::Get())>>) return Reflect<Type>::Get();\
		else return nullptr;\
	}\
}

/** Define standard chained member functions used to build a TypeInfo instance */
#define TYPEINFO_BUILDER_METHODS(Type)\
using TypeInfo::description; using TypeInfo::flags;\
inline decltype(auto) Description(std::string_view inDescription) { description = inDescription; return *this; }\
inline decltype(auto) Flags(Reflection::FTypeFlags inFlags) { flags += inFlags; return *this; }\
