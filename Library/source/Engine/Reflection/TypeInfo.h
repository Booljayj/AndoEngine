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
			if constexpr (std::is_default_constructible_v<Type>) flags += ETypeFlags::DefaultConstructable;
			if constexpr (std::is_copy_constructible_v<Type>) flags += ETypeFlags::CopyConstructable;
			if constexpr (std::is_copy_assignable_v<Type>) flags += ETypeFlags::CopyAssignable;
			if constexpr (HasOperatorEquals_V<Type>) flags += ETypeFlags::EqualityComparable;
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

	/** A library which contains types. Corresponds to a particular DLL or EXE created by the linker. */
	struct TypeLibrary {
	public:
		friend class TypeInfo;
		using TypeInfoCollectionType = std::deque<TypeInfo const*>;
		using value_type = TypeInfoCollectionType::value_type;

		/** The name of the libraries, which should correspond to the DLL or EXE name */
		std::string_view name;

		static inline TypeLibrary const& Local() { return LocalMutable(); }

		TypeInfoCollectionType::const_iterator cbegin() const { return types.cbegin(); }
		TypeInfoCollectionType::const_iterator cend() const { return types.cend(); }

	private:
		/** Types that are defined within this library */
		TypeInfoCollectionType types;

		TypeLibrary(std::string_view inName) : name(inName) {}

		static inline TypeLibrary& LocalMutable() {
			static TypeLibrary library{ std::string_view{ STRINGIFY(TARGET_NAME) } };
			return library;
		}

		static inline void Register(TypeInfo const* type) { LocalMutable().types.push_back(type); }
	};

	/** Provides a set of runtime information about a type */
	struct TypeInfo {
	public:
		using TypeInfoCollection = std::deque<TypeInfo const*>;

		static constexpr ETypeClassification Classification = ETypeClassification::Unknown;

		/** The classification of this TypeInfo, defining what kinds of type information it contains */
		ETypeClassification classification = ETypeClassification::Primitive;
		/** The identifier for this type. Always unique and stable. */
		Hash128 id;
		/** The base name of this type, without template parameters but including scopes */
		std::string_view name;
		/** Flags that provide additional information about this type */
		FTypeFlags flags;
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

		/** Convert this TypeInfo to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
		template<typename TypeInfoType>
		TypeInfoType const* As() const {
			static_assert(std::is_base_of_v<TypeInfo, TypeInfoType>, "TypeInfo::As may only convert to types that derive from TypeInfo");
			if (TypeInfoType::Classification == classification) return static_cast<TypeInfoType const*>(this);
			else return nullptr;
		}

	protected:
		inline TypeInfo(ETypeClassification inClassification, Hash128 inID, std::string_view inName, FTypeFlags inFlags, MemoryParams inMemory)
			: classification(inClassification), id(inID), name(inName), flags(inFlags), memory(inMemory)
		{
			//The library and the contained types will always be loaded together, so we don't need to remove these types from the container when destroying
			TypeLibrary::Register(this);
		}
	};

	/** Implements type-specific operation overrides for a particular TypeInfo instance. */
	template<typename Type, typename TypeInfoType>
	struct ImplementedTypeInfo : public TypeInfoType {
		static constexpr Type const& Cast(void const* pointer) { return *static_cast<Type const*>(pointer); }
		static constexpr Type& Cast(void* pointer) { return *static_cast<Type*>(pointer); }

		ImplementedTypeInfo(Hash128 inID, std::string_view inName)
			: TypeInfoType(TypeInfoType::Classification, inID, inName, FTypeFlags::Create<Type>(), MemoryParams::Create<Type>())
		{}

		virtual void Destruct(void* instance) const final {
			static_assert(std::is_destructible_v<Type>, "Reflected types must always be destructible");
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
			if constexpr (HasOperatorEquals_V<Type>) return Cast(a) == Cast(b);
			else return false;
		}
	};

	/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
	template<typename TypeInfoType>
	TypeInfoType const* Cast(TypeInfo const* info) {
		static_assert(std::is_base_of_v<TypeInfo, TypeInfoType>, "TypeInfo::Cast may only convert to types that derive from TypeInfo");
		if (info) return info->As<TypeInfoType>();
		else return nullptr;
	}
}

/** Struct which provides reflection information for a type. Must be specialized for all reflected types. Non-reflected types return default values. */
template<typename Type>
struct Reflect {
	//static ::Reflection::TypeInfo const* Get() { return nullptr; }
	//static constexpr Hash128 ID = Hash128{};
};

/** Define a Reflect implementation that only resolves if the type uses {Name}TypeInfo. Equal in behavior to calling Cast<{Name}TypeInfo>(Reflect<Type>::Get()). */
#define TYPEINFO_REFLECT(Name)\
template<typename Type>\
struct Reflect ## Name {\
	static ::Reflection::Name ## TypeInfo const* Get() {\
		if constexpr (std::is_base_of_v<::Reflection::Name ## TypeInfo, std::remove_pointer_t<decltype(Reflect<Type>::Get())>>) return Reflect<Type>::Get();\
		else return nullptr;\
	}\
}

/** Define standard chained member functions used to build a TypeInfo instance */
#define TYPEINFO_BUILDER_METHODS(Type)\
using TypeInfo::description; using TypeInfo::flags;\
inline decltype(auto) Description(std::string_view inDescription) { description = inDescription; return *this; }\
inline decltype(auto) Flags(Reflection::FTypeFlags inFlags) { flags += inFlags; return *this; }\
