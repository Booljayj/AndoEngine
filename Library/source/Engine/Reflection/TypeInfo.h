#pragma once
#include "Engine/Concepts.h"
#include "Engine/Core.h"
#include "Engine/Flags.h"
#include "Engine/FunctionRef.h"
#include "Engine/Hash.h"
#include "Engine/StringView.h"
#include "ThirdParty/yaml.h"

namespace Reflection {
	struct TypeInfo;
	struct ValuelessTypeInfo;
	struct NumericTypeInfo;
	struct StringTypeInfo;
	struct StructTypeInfo;
	struct EnumTypeInfo;
	struct FlagsTypeInfo;
	struct ArrayTypeInfo;
	struct MapTypeInfo;
	struct SetTypeInfo;
	struct PolyTypeInfo;
	struct ReferenceTypeInfo;
	struct TupleTypeInfo;
	struct VariantTypeInfo;
}

/** Struct which provides reflection information for a type. Must be specialized for all reflected types. Non-reflected types fail to compile if this is used. */
template<typename Type>
struct Reflect {
	//static ::Reflection::TypeInfo const& Get() { return Type::info; }
	//static constexpr Hash128 ID = Hash128{};
};

bool operator==(::Reflection::TypeInfo const& a, ::Reflection::TypeInfo const& b);

/** Declare a reflect implementation for a qualified type */
#define REFLECT(QualifiedType, Classification)\
template<> struct ::Reflect<QualifiedType> {\
	static ::Reflection::Classification ## TypeInfo const& Get();\
	static constexpr Hash128 ID = STRINGIFY(QualifiedType) ## _h128;\
}

namespace Reflection {
	/** Classifications for types. Each classification contains types that share a similar usage or format. */
	enum class ETypeClassification : uint8_t {
		/** An uknown type. Only basic methods and information are available, it must be cast to a more specific type to do anything else. */
		Unknown,

		//Basic construct types
		/** The type represents a valueless entity, such as void or monostate */
		Valueless,
		/** The type represents a number value */
		Numeric,
		/** The type is a string of characters */
		String,
		/** The type is a collection of named values and methods */
		Struct,
		/** The type represents a value that equal to a discrete named constant within a collection of discrete named constants */
		Enum,
		/** The type represents a collection of values that are all equal to a discrete named constant within a collection of discrete named constants */
		Flags,

		//Homogeneous collection types
		/** The type is a collection of elements, where each element has the same type */
		Array,
		/** The type is a collection of key-value pairs, where each key and each value has the same type and no duplicates are allowed within the keys */
		Map,
		/** The type is a collection of elements, where each element has the same type and no duplicates are allowed within the collection */
		Set,

		//Heterogeneous collection types
		/** The type represents a struct value that derives from a known base struct. The value is owned by this type. */
		Poly,
		/** The type represents a struct value that derives from a known base struct. The value is owned externally, and referenced by this type. */
		Reference,
		/** The type represents a collection of unnamed values, which may have different types */
		Tuple,
		/** The type represents a single value that has a type that is one of a collection of known types */
		Variant,
	};
	
	/** flags that describe traits of a particular type */
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
	};

	/** flags that describe traits of a particular type */
	struct FTypeFlags : public TFlags<ETypeFlags> {
		TFLAGS_METHODS(FTypeFlags);

		/** Create a standard set of flags based on the type */
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
			if constexpr (std::is_void_v<Type>) return MemoryParams{ 0, 0 };
			else return MemoryParams{ sizeof(Type), alignof(Type) };
		};
	};

	/** Provides a set of runtime information about a type */
	struct TypeInfo {
		static constexpr ETypeClassification Classification = ETypeClassification::Unknown;

		/** The classification of this TypeInfo, defining what kinds of type information it contains */
		ETypeClassification const classification = ETypeClassification::Unknown;
		
		/** The identifier for this type. Always unique and stable. */
		Hash128 const id;
		/** The memory parameters for this type */
		MemoryParams const memory;
		/** Flags that provide additional information about this type */
		FTypeFlags const flags;

		/** The fully-qualified name of this type. If this is a template, does not include template parameters. */
		std::u16string_view const name;
		/** Human-readable description of this type */
		std::u16string_view const description;

		TypeInfo() = delete;

		//@todo This does not respect alignment, and it needs to. Using aligned allocation methods means also using aligned deallocation methods, so the unique pointer needs
		//      a custom deleter (using default_delete is malformed).
		/** Allocate uninitialized memory large enough to hold an instance of this type, and return a unique pointer to that memory */
		inline std::unique_ptr<std::byte[]> AllocateUninitialized() const { return std::unique_ptr<std::byte[]>{ new std::byte[memory.size] }; }
		
		/** Return the fully-qualified name of this type, including template parameters if the type is an instantiation of a template. */
		virtual std::u16string GetName() const { return std::u16string{ name }; }

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

		/** Serialize an instance of this type in binary format */
		virtual void Serialize(Archive::Output& archive, void const* instance) const = 0;
		/** Deserialize an instance of this type in binary format */
		virtual void Deserialize(Archive::Input& archive, void* instance) const = 0;

		/** Serialize an instance of this type in YAML format */
		virtual YAML::Node Serialize(void const* instance) const = 0;
		/** Deserialize an instance of this type in YAML format */
		virtual void Deserialize(YAML::Node const& node, void* instance) const = 0;

		/** Builder method to add flags to a type */
		template<typename Self>
		inline auto& Flags(this Self&& self, Reflection::FTypeFlags inFlags) { self.flags += inFlags; return self; }
		
	protected:
		inline TypeInfo(ETypeClassification classification, Hash128 id, MemoryParams memory, FTypeFlags flags, std::u16string_view name, std::u16string_view description)
			: classification(classification), id(id), memory(memory), flags(flags), name(name), description(description)
		{}
	};

	namespace Concepts {
		template<typename T>
		concept ReflectedType = requires {
			{ ::Reflect<T>::Get() } -> std::convertible_to<TypeInfo const&>;
			{ ::Reflect<T>::ID } -> std::convertible_to<Hash128>;
		};

		template<typename T>
		concept HasGetTypeInfoMethod = std::is_class_v<T> and requires(T a) {
			{ a.GetTypeInfo() } -> std::convertible_to<StructTypeInfo const&>;
		};

#define REFLECTED_TYPE_CONCEPT(Classification) \
		template<typename T> concept Reflected ## Classification ## Type = ReflectedType<T> and requires { \
			{ ::Reflect<T>::Get() } -> std::convertible_to<Classification ## TypeInfo const&>; \
		}

		REFLECTED_TYPE_CONCEPT(Valueless);
		REFLECTED_TYPE_CONCEPT(Numeric);
		REFLECTED_TYPE_CONCEPT(String);
		REFLECTED_TYPE_CONCEPT(Struct);
		REFLECTED_TYPE_CONCEPT(Enum);
		REFLECTED_TYPE_CONCEPT(Flags);
		REFLECTED_TYPE_CONCEPT(Array);
		REFLECTED_TYPE_CONCEPT(Map);
		REFLECTED_TYPE_CONCEPT(Set);
		REFLECTED_TYPE_CONCEPT(Poly);
		REFLECTED_TYPE_CONCEPT(Reference);
		REFLECTED_TYPE_CONCEPT(Tuple);
		REFLECTED_TYPE_CONCEPT(Variant);

#undef REFLECTED_TYPE_CONCEPT
	}

	/** Info for a valueless type */
	struct ValuelessTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Valueless;
	};

	/** Info for a numeric type, which can be assigned from various number values and retrieved as a number value. */
	struct NumericTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Numeric;

		enum struct ENumericType : uint8_t {
			UnsignedInteger,
			SignedInteger,
			FloatingPoint,
			Binary,
			Bits,
		};

		/** The type of number that this represents, which can determine what kinds of values it will hold. */
		ENumericType numeric_type = ENumericType::UnsignedInteger;

		virtual uint64_t GetUnsignedInteger(void const* instance) const = 0;
		virtual int64_t GetSignedInteger(void const* instance) const = 0;
		virtual double GetFloatingPoint(void const* instance) const = 0;

		virtual void Set(void* instance, uint64_t value) const = 0;
		virtual void Set(void* instance, int64_t value) const = 0;
		virtual void Set(void* instance, double value) const = 0;
	};

	/** Info for a string type, which is a sequence of characters */
	struct StringTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::String;

		/** The type for the characters within the string */
		NumericTypeInfo const* characters = nullptr;

		/** Get the contents of the string as UTF-16 */
		virtual std::u16string Get(void const* instance) const = 0;
		/** Set the contents of the string from UTF-16 */
		virtual void Set(void const* instance, std::u16string const& string) const = 0;
		/** Reset the contents of the string, so it will be empty */
		virtual void Reset() const = 0;
	};

	/** flags to describe traits of a particular variable */
	enum class EVariableFlags : uint32_t {
		/** The variable cannot be modified */
		Const,
		/** The variable is global, and does not belong to a specific instance. May be accessed with a null instance pointer. */
		Static,
		/** The variable should not be shown when displaying variables to a user */
		Hidden,
		/** The variable should not be serialized when serializing the containing struct */
		NonSerialized,
		/** The variable is deprecated. Existing values will be loaded during serialization, but new values will not be saved. Using the variable should produce a warning. */
		Deprecated,
	};
	using FVariableFlags = TFlags<EVariableFlags>;

	/** Info that describes a variable value within a struct */
	struct VariableInfo {
		TypeInfo const* type = nullptr;
		Hash32 id;
		std::u16string_view name;
		std::u16string_view description;
		FVariableFlags flags;

		virtual ~VariableInfo() = default;

		virtual void const* GetImmutable(void const* instance) const = 0;
		virtual void* GetMutable(void* instance) const = 0;

	protected:
		VariableInfo(TypeInfo const* type, Hash32 id, std::u16string_view name, std::u16string_view description, FVariableFlags flags)
			: type(type), id(id), name(name), description(description), flags(flags)
		{}
	};

	/** Info for a struct type, which contains various fields and supports inheritance */
	struct StructTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Struct;

		/** The type that this type inherits from. Only single-inheritance from another object type is supported. */
		StructTypeInfo const* base = nullptr;

		static void SerializeVariables(StructTypeInfo const& type, YAML::Node& node, void const* instance);
		static void DeserializeVariables(StructTypeInfo const& type, YAML::Node const& node, void* instance);

		static void SerializeVariables(StructTypeInfo const& type, Archive::Output& archive, void const* instance);
		static void DeserializeVariables(StructTypeInfo const& type, Archive::Input& archive, void* instance);

		/** Allocate an default-construct an instance of this type. If the type cannot be default-constructed, this returns an empty pointer. */
		template<Concepts::ReflectedStructType T>
		inline std::unique_ptr<T> Allocate() {
			if (!IsChildOf<T>()) throw std::runtime_error{ "Invalid allocation to non-parent class pointer" };
			return std::unique_ptr<T>(reinterpret_cast<T*>(AllocateRaw()));
		}

		bool IsChildOf(StructTypeInfo const& parent) const {
			//Walk up the chain of parents until we encounter the provided type
			for (StructTypeInfo const* current = this; current; current = current->base) {
				if (current == &parent) return true;
			}
			return false;
		}

		template<Concepts::ReflectedStructType T>
		bool IsChildOf() const { return IsChildOf(Reflect<T>::Get()); }

		/** Get a span of the variables contained in this struct */
		virtual std::span<std::unique_ptr<VariableInfo const> const> GetVariables() const = 0;
		/** Get a default-constructed instance of this struct. Will return nullptr if the struct cannot be default-constructed. */
		virtual void const* GetDefaults() const = 0;

		/** Returns the known specific type of an instance deriving from this type, to our best determination. If this cannot be determined, this method will return the same type. */
		virtual StructTypeInfo const& GetInstanceTypeInfo(void const* instance) const = 0;

	protected:
		virtual void* AllocateRaw() const = 0;

	private:
		static void SerializeVariables_Diff(StructTypeInfo const& type, YAML::Node& node, void const* instance, void const* defaults);
		static void SerializeVariables_Diff(StructTypeInfo const& type, Archive::Output& archive, void const* instance, void const* defaults);

		static void SerializeVariables_NonDiff(StructTypeInfo const& type, YAML::Node& node, void const* instance);
		static void SerializeVariables_NonDiff(StructTypeInfo const& type, Archive::Output& archive, void const* instance);
	};

	/** TypeInfo for an enum, which is a type that can be set equal to one of several discrete named values */
	struct EnumTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		using TypeInfo::GetName;
		static constexpr ETypeClassification Classification = ETypeClassification::Enum;

		/** The underlying type of the values in the enumeration */
		TypeInfo const* underlying = nullptr;

		virtual void Serialize(YAML::Node& node, void const* instance) const final { node = GetName(GetIndexOfValue(instance)); }
		virtual void Deserialize(YAML::Node const& node, void* instance) const final { underlying->Copy(instance, GetValue(GetIndexOfName(node.as<std::string>()))); }

		/** Get the number of values that the enumeration defines */
		virtual size_t GetCount() const = 0;

		/** Get the value at the specified index */
		virtual void const* GetValue(size_t index) const = 0;
		/** Get the name of the value at the specified index */
		virtual std::string_view GetName(size_t index) const = 0;

		/** Get the index of the first element with the specified value */
		virtual size_t GetIndexOfValue(void const* value) const = 0;
		/** Get the index of the first element with the specified name */
		virtual size_t GetIndexOfName(std::string_view name) const = 0;
	};

	/**
	 * Flags are a type wherein several components of some underlying type can be combined
	 * together into an aggregate with the same underlying type. The components must have
	 * unique values and names, and several aggregates may be predefined if appropriate.
	 */
	struct FlagsTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Flags;

		TypeInfo const* underlying = nullptr;

		/** Get the number of components that the flags have */
		virtual size_t GetComponentCount() const = 0;
		/** Get the number of predefined aggregates that the flags have */
		virtual size_t GetAggregateCount() const = 0;

		/** Get the name of the component at the specified index */
		virtual std::string_view GetComponentName(size_t index) const = 0;
		/** Get the value of the component at the specified index */
		virtual void const* GetComponentValue(size_t index) const = 0;
		/** Get the name of the aggregate at the specified index */
		virtual std::string_view GetAggregateName(size_t index) const = 0;
		/** Get the value of the aggregate at the specified index */
		virtual void const* GetAggregateValue(size_t index) const = 0;

		/** Get the index of the component with the specified name */
		virtual size_t GetIndexOfComponentName(std::string_view name) const = 0;
		/** Get the index of the component with the specified value */
		virtual size_t GetIndexOfComponentValue(void const* value) const = 0;
		/** Get the index of the first aggregate with the specified name */
		virtual size_t GetIndexOfAggregateName(std::string_view name) const = 0;
		/** Get the index of the first aggregate with the specified value */
		virtual size_t GetIndexOfAggregateValue(void const* value) const = 0;

		/** Get the name of the special "empty" aggregate, which contains no components */
		virtual std::string_view GetEmptyName() const = 0;
		/** Get the value of the special "empty" aggregate, which contains no components */
		virtual void const* GetEmptyValue() const = 0;

		/** Remove all components from the aggregate value */
		virtual void Reset(void* aggregate) const = 0;
		/** Add the component or aggregate value to the aggregate */
		virtual void Add(void* aggregate, void const* value) const = 0;
		/** Subtract the component or aggregate value from the aggregate */
		virtual void Subtract(void* aggregate, void const* value) const = 0;
		/** Returns true if the aggregate contains the component or aggregate value */
		virtual bool Contains(void const* aggregate, void const* value) const = 0;
		/** Returns true if the aggregate is empty (contains no components) */
		virtual bool IsEmpty(void const* aggregate) const = 0;

		/** Fills the output vector with the indices of any components in the aggregate */
		virtual void ForEachComponent(void const* aggregate, FunctionRef<bool(size_t)> functor) const = 0;
	};

	/** TypeInfo for an array type. Arrays are containers of elements of the same type. */
	struct ArrayTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Array;

		/** Whether the number of elements in the array can be manipulated */
		bool isFixedSize = false;
		/** The type of the elements in the array */
		TypeInfo const* elements = nullptr;

		/** Invokes the functor with a pointer to each element in the array and its index. Stops if the functor returns false. */
		virtual void ForEachElement(void* instance, FunctionRef<bool(void*, size_t)> functor) const = 0;
		virtual void ForEachElement(void const* instance, FunctionRef<bool(void const*, size_t)> functor) const = 0;

		/** Get the number of elements that are in the array */
		virtual size_t GetCount(void const* instance) const = 0;

		/** Get a pointer to a specific element in the array. The pointer should be considered invalid if the array is modified in any way. */
		virtual void* GetElement(void* instance, size_t index) const = 0;
		virtual void const* GetElement(void const* instance, size_t index) const = 0;

		/** Resize the array to hold a specific number of elements. Returns true if successful. */
		virtual bool Resize(void* instance, size_t count) const = 0;

		/** Remove all elements in the container. Returns true if successful. */
		virtual bool ClearElements(void* instance) const = 0;
		/** Add a new elementType to the "end" of the array. Returns true if successful. */
		virtual bool AddElement(void* instance, void const* value) const = 0;

		/** Remove the pointed-at elementType. Returns true if successful. */
		virtual bool RemoveElement(void* instance, void const* element) const = 0;
		/** Insert a new element at the position of the pointed-at element, equal to the value. If value is nullptr, the new element is default-constructed. Returns true if successful. */
		virtual bool InsertElement(void* instance, void const* element, void const* value) const = 0;
	};

	/** TypeInfo for a map type. Maps are containers of key-value pairs with unique keys. */
	struct MapTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Map;

		/** The type of the keys in the map */
		TypeInfo const* keys = nullptr;
		/** The type of the values in the map */
		TypeInfo const* values = nullptr;

		/** Get the number of entries in this map */
		virtual size_t GetCount(void const* instance) const = 0;

		/** Call the functor with each pair in the map. Stops when the functor returns false. */
		virtual void ForEachPair(void* instance, FunctionRef<bool(std::pair<void const*, void*>)> functor) const = 0;
		virtual void ForEachPair(void const* instance, FunctionRef<bool(std::pair<void const*, void const*>)> functor) const = 0;

		/** Find the value for a key. Returns nullptr if the key is not in the map */
		virtual void* Find(void* instance, void const* key) const = 0;
		virtual void const* Find(void const* instance, void const* key) const = 0;

		/** Remove all entries from the map */
		virtual void Clear(void* instance) const = 0;

		/** Remove the entry corresponding to a particular key from the map. Returns true if successful. */
		virtual bool RemoveEntry(void* instance, void const* key) const = 0;
		/** Add a new entry to the map with the specified value. If the key is already in the map, nothing will happen and will return false. If value is nullptr, the new value will be default constructed */
		virtual bool InsertEntry(void* instance, void const* key, void const* value) const = 0;
	};

	/** TypeInfo for a set type. Sets are collections of unique elements. */
	struct SetTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Set;

		/** The type of the values in the set */
		TypeInfo const* values = nullptr;

		/** Get the number of values that are in the set */
		virtual size_t GetCount(void const* instance) const = 0;

		/** Call a functor for each value in the set. Stops when the functor returns false. */
		virtual void ForEachElement(void const* instance, FunctionRef<bool(void const*)> functor) const = 0;

		/** Returns true if an element with an equal value is contained in the set */
		virtual bool Contains(void const* instance, void const* value) const = 0;

		/** Remove all values from the set */
		virtual void Clear(void* instance) const = 0;
		/** Adds the value to the set */
		virtual bool Add(void* instance, void const* value) const = 0;
		/** Remove the value from the set */
		virtual bool Remove(void* instance, void const* value) const = 0;
	};

	/** TypeInfo for a poly type. "Poly" is short for Polymorphic Interface. A poly owns a pointer to an optional instance that can be cast to a base type. */
	struct PolyTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Poly;

		/** The base type for this poly */
		StructTypeInfo const* base = nullptr;

		inline bool CanAssignType(StructTypeInfo const& type) const {
			if (type == *base) return !type.flags.Has(ETypeFlags::Abstract);
			else if (type.IsChildOf(*base)) return true;
			else return false;
		}

		/** Get the current value of the poly. Can be nullptr if the poly is unassigned */
		virtual void* GetValue(void* instance) const = 0;
		virtual void const* GetValue(void const* instance) const = 0;

		/** Get the current type of the poly. Can be nullptr if the poly is unassigned, otherwise can be anything deriving from base. */
		virtual StructTypeInfo const* GetType(void const* instance) const = 0;

		/** Assign a new value to a poly */
		virtual bool Assign(void* instance, StructTypeInfo const& type, void const* value) const = 0;
		/** Unassign the value of a poly */
		virtual void Unassign(void* instance) const = 0;
	};

	/**
	 * TypeInfo for a reference type. Reference types hold a reference to some externally-owned object.
	 * A reference is always a shared_ptr or weak_ptr, which implement the referencing mechanics.
	 * The referenced object must also be a reflected struct type, to ensure we can safely retrieve information about it.
	 */
	struct ReferenceTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Reference;

		/** The base type of the referenced object. The actual reference may be a derived type. */
		StructTypeInfo const* base = nullptr;
		/** True if the referenced object is immutable when accessed through this reference */
		bool isImmutable = false;

		/** Get a type-erased reference to the object */
		virtual std::shared_ptr<void> GetMutable(void const* instance) const = 0;
		/** Get a type-erased reference to the object */
		virtual std::shared_ptr<void const> GetImmutable(void const* instance) const = 0;

		/** Reset an instance so it no longer holds a reference to any object */
		virtual void Reset(void* instance) = 0;
	};

	/** TypeInfo for a tuple type. Tuples are an indexed collection of other types. */
	struct TupleTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Tuple;

		/** The types of each element in the tuple */
		virtual std::span<TypeInfo const*> GetTypes() const = 0;

		/** Get the value at a specific index in the tuple */
		virtual void* GetValue(void* instance, size_t index) const = 0;
		virtual void const* GetValue(void const* instance, size_t index) const = 0;
	};

	/** TypeInfo for a variant type. Variants are containers that can hold a single value with more than one possible type. */
	struct VariantTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Variant;

		/** Get the possible types this variant can have */
		virtual std::span<TypeInfo const*> GetTypes() const = 0;

		/** Returns the current type the variant is holding */
		virtual TypeInfo const& GetType(void const* instance) const = 0;

		/** Returns the value in the variant */
		virtual void* GetValue(void* instance) const = 0;
		virtual void const* GetValue(void const* instance) const = 0;

		/** Assign the value inside the variant. Returns true if the new value was successfully assigned. Value is optional, if provided the newly assigned value will be a copy */
		virtual bool Assign(void* instance, const TypeInfo& type, void const* source) const = 0;
	};
}

//============================================================
// Utility methods and aliases added to the global namespace for convenience

inline bool operator==(::Reflection::TypeInfo const& a, ::Reflection::TypeInfo const& b) { return std::addressof(a) == std::addressof(b) || a.id == b.id; }

/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
template<std::derived_from<Reflection::TypeInfo> TargetType, std::derived_from<Reflection::TypeInfo> OriginalType>
TargetType const* Cast(OriginalType const* info) {
	//Upcasting or the same type does not need any special logic, so we can return the type directly
	if constexpr (std::derived_from<OriginalType, TargetType>) return static_cast<TargetType const*>(info);
	//Otherwise, check if the type's classification value matches the classification of TargetType
	if (info && info->classification == TargetType::Classification) return static_cast<TargetType const*>(info);
	else return nullptr;
}

/** Convert a TypeInfo pointer to a specific kind of TypeInfo. Will return nullptr if the conversion is not possible */
template<std::derived_from<Reflection::TypeInfo> TargetType, std::derived_from<Reflection::TypeInfo> OriginalType>
TargetType const* Cast(OriginalType const& info) {
	//Upcasting or the same type does not need any special logic, so we can return the type directly
	if constexpr (std::derived_from<OriginalType, TargetType>) return static_cast<TargetType const*>(&info);
	//Otherwise, check if the type's classification value matches the classification of TargetType
	if (info.classification == TargetType::Classification) return static_cast<TargetType const*>(&info);
	else return nullptr;
}

//============================================================
// Implementation helpers and standard implementations

namespace Reflection {
	/** Implements type-specific operation overrides for a particular TypeInfo instance. */
	template<std::destructible Type, std::derived_from<TypeInfo> BaseType>
	struct ImplementedTypeInfo : public BaseType {
		static constexpr Type const& Cast(void const* pointer) { return *static_cast<Type const*>(pointer); }
		static constexpr Type& Cast(void* pointer) { return *static_cast<Type*>(pointer); }

		ImplementedTypeInfo(Hash128 id, std::u16string_view name, std::u16string_view description)
			: BaseType(BaseType::Classification, id, MemoryParams::Create<Type>(), FTypeFlags::Create<Type>(), name, description)
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

		virtual void Serialize(Archive::Output& archive, void const* instance) const override { return Archive::Serializer<Type>::Write(archive, Cast(instance)); }
		virtual void Deserialize(Archive::Input& archive, void* instance) const override { return Archive::Serializer<Type>::Read(archive, Cast(instance)); }

		virtual YAML::Node Serialize(void const* instance) const override { return YAML::convert<Type>::encode(Cast(instance)); }
		virtual void Deserialize(YAML::Node const& node, void* instance) const override { YAML::convert<Type>::decode(node, Cast(instance)); }
	};
}

REFLECT(void, Valueless);
REFLECT(std::monostate, Valueless);

REFLECT(std::byte, Numeric);
REFLECT(bool, Numeric);

REFLECT(int8_t, Numeric);
REFLECT(uint8_t, Numeric);
REFLECT(int16_t, Numeric);
REFLECT(uint16_t, Numeric);
REFLECT(int32_t, Numeric);
REFLECT(uint32_t, Numeric);
REFLECT(int64_t, Numeric);
REFLECT(uint64_t, Numeric);

REFLECT(char, Numeric);
REFLECT(char8_t, Numeric);
REFLECT(char16_t, Numeric);
REFLECT(char32_t, Numeric);

REFLECT(float, Numeric);
REFLECT(double, Numeric);
