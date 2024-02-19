#pragma once
#include "Engine/InlineInterface.h"
#include "Engine/StandardTypes.h"
#include "Engine/TypeTraits.h"

//Delegates wrap a callable together with an optional context object. When executed, the callable
//and context object are used together to invoke the specified behavior. Based on "The Impossibly
//Fast ObjectType++ Delegates" by Sergey Ryazanov and similar resources, but heavily modified to support
//a larger feature set and reduce allocations.
//https://www.codeproject.com/articles/11015/the-impossibly-fast-c-delegates

//Delegates are defined using the signature of the callable, and are created using static methods
//on that specialization. For example:
//- Delegate has signature void(), and is bound to a free function that has that signature
//		TDelegate<void> D = TDelegate<void>::Create( &FreeFunction );
//- Delegate has signature int(std::string), and is bound to a method that has that signature
//		ClassType instance;
//		TDelegate<int, std::string> D = TDelegate<int, std::string>::Create( &instance, &ClassType::Method );
//- Delegate has signature void(char), and is bound to a lambda that also accepts the context object
//		ClassType instance;
//		TDelegate<void, char> D = TDelegate<void, char>::Create( &instance, [](ClassType* I){ /*...*/ } );

//Some notes on usage:
//- Delegate reassignment is allowed using the copy and move assignment operators
//- Delegates can be default constructed, executing a default delegate will do nothing and return a
//	default-constructed return value.
//- Bound parameters are supported by providing a lambda that performs a capture. All lambda types are
//	supported.
//- In most cases, no allocations are performed when binding the delegate. Lambdas with a large number
//	of captured values can cause an allocation to store a copy of the lambda.
//- DelegateBase exists to move some method and type declarations outside of template specializations,
//  which reduces the binary size a bit when using delegates. They still require a lot of specialized
//  compiled binary code for each unique template specialization.

//Known issues
//- The semantics objects are statically initialized, but no safety is performed if those objects are
//  created across different dlls. It is therefore very unsafe to copy delegates across dll boundaries.
//  The ideal solution would be to either ensure semantics objects are shared between dlls, or create
//  the right dll-specific semantics object during copy and move operations. The first option requires
//  specialized linker markup, and the second option requires some very fancy double-blind type erasure.

struct DelegateBase {
protected:
	/** The type used for the delegate's context object, if one exists */
	union DelegateContext {
		void* rawPointer;
		void const* constRawPointer;
		std::weak_ptr<void> smartPointer;
		std::weak_ptr<void const> constSmartPointer;

		//Types are managed by the semantics objects, so this union is left uninitialized
		DelegateContext() {};
		~DelegateContext() {}
	};
	/** The type used to store the actual callable object wrapped by this delegate. */
	union DelegateStorage {
		void* external;
		alignas(PointerTraits::FunctionPointerAlign) std::byte internal[PointerTraits::FunctionPointerSize];

		//Types are managed by the semantics objects, so this union is left uninitialized
		DelegateStorage() {};
		~DelegateStorage() {}
	};

	/** Helper template to determine if a type can fit in internal storage */
	template<typename T>
	static constexpr bool IsInternal() { return sizeof(T) <= sizeof(DelegateStorage::internal); }

	/** Templates that are specialized with semantics for various types, to ensure proper behavior when constructing, destructing, and comparing them. */
	template<typename ActualContextType>
	struct TContextSemanticsImpl;
	template<typename ActualStorageType>
	struct TStorageSemanticsImpl;

	/** Base interface for semantics, which erases type information but handles basic operations */
	struct IDelegateSemantics : public TInlineClonable<IDelegateSemantics> {
		virtual ~IDelegateSemantics() = default;
		virtual void Clone(IDelegateSemantics* destination) const { new (destination) IDelegateSemantics(); };
		virtual bool IsRawPointer() const noexcept { return true; }
		virtual void CopyConstruct(DelegateBase& target, DelegateBase const& source) const noexcept {}
		virtual void MoveConstruct(DelegateBase& target, DelegateBase&& source) const noexcept {}
		virtual void Destruct(DelegateBase& target) const noexcept {}
		virtual bool IsValid(DelegateBase const& target) const noexcept { return true; }
		virtual bool IsBoundTo(DelegateBase const& target, void* object) const noexcept { return false; }
		virtual bool IsEqual(DelegateBase const& target, DelegateBase const& other) const noexcept { return true; }
	};

	/** Wraps the semantics for the context and storage into the generic IDelegateSemantics interface */
	template<typename ContextSemanticsImplType, typename StorageSemanticsImplType>
	struct TWrappedSemantics : public IDelegateSemantics {
		virtual void Clone(IDelegateSemantics* destination) const { new (destination) TWrappedSemantics(); };
		virtual bool IsRawPointer() const noexcept final { return ContextSemanticsImplType::IsRawPointerType; }
		virtual void CopyConstruct(DelegateBase& target, const DelegateBase& source) const noexcept final {
			ContextSemanticsImplType::CopyConstruct(target.context, source.context);
			StorageSemanticsImplType::CopyConstruct(target.storage, source.storage);
		}
		virtual void MoveConstruct(DelegateBase& target, DelegateBase&& source) const noexcept final {
			ContextSemanticsImplType::MoveConstruct(target.context, std::move(source.context));
			StorageSemanticsImplType::MoveConstruct(target.storage, std::move(source.storage));
		}
		virtual void Destruct(DelegateBase& target) const noexcept final {
			ContextSemanticsImplType::Destruct(target.context);
			StorageSemanticsImplType::Destruct(target.storage);
		}

		virtual bool IsValid(DelegateBase const& target) const noexcept final {
			return ContextSemanticsImplType::IsValid(target.context);
		}
		virtual bool IsBoundTo(DelegateBase const& target, void* object) const noexcept final {
			return ContextSemanticsImplType::IsBoundTo(target.context, object);
		}
		virtual bool IsEqual(DelegateBase const& target, DelegateBase const& other) const noexcept final {
			return ContextSemanticsImplType::IsContextEqual(target.context, other.context) && StorageSemanticsImplType::IsStorageEqual(target.storage, other.storage);
		}
	};

	TInlineInterface<IDelegateSemantics> semantics;
	DelegateContext context;
	DelegateStorage storage;

	DelegateBase() = default; //Protected constructor ensures this cannot be instantiated directly
	
	template<typename ContextType, typename StorageType>
	DelegateBase(std::in_place_type_t<ContextType>, std::in_place_type_t<StorageType>) 
		: semantics(std::in_place_type<TWrappedSemantics<TContextSemanticsImpl<ContextType>, TStorageSemanticsImpl<StorageType>>>)
	{}

	DelegateBase(const DelegateBase& other)
		: semantics(other.semantics)
	{}

public:
	virtual ~DelegateBase() {}
};

template<typename ReturnType, typename... ParamTypes>
struct TDelegate : public DelegateBase {
private:
	/** Executor used to invoke the bound operation with the correct semantics using a generic interface */
	using ExecutorType = ReturnType(*)(DelegateContext const&, DelegateStorage const&, ParamTypes...);
	ExecutorType executor;

public:
	TDelegate() noexcept : DelegateBase(std::in_place_type<void>, std::in_place_type<void>) {
		executor = [](DelegateContext const&, DelegateStorage const&, ParamTypes...) {};
	}
	~TDelegate() noexcept {
		executor = nullptr;
	}

	TDelegate(TDelegate const& other) noexcept : DelegateBase(other) {
		executor = other.executor;
		semantics->CopyConstruct(*this, other);
	}
	TDelegate(TDelegate&& other) noexcept : DelegateBase(other) {
		executor = other.executor;
		semantics->MoveConstruct(*this, std::move(other));
	}
	TDelegate& operator=(TDelegate const& other) noexcept {
		semantics = other.semantics;
		executor = other.executor;
		semantics->CopyConstruct(*this, other);
		return *this;
	}
	TDelegate& operator=(TDelegate&& other) noexcept {
		semantics = other.semantics;
		executor = other.executor;
		semantics->MoveConstruct(*this, std::move(other));
		return *this;
	}

	/** Two delegates are equal only if they point to the same instance and the same method, or if they are both empty. */
	friend inline bool operator==(TDelegate const& a, TDelegate const& b) { return a.semantics->IsEqual(a, b); }
	friend inline bool operator!=(TDelegate const& a, TDelegate const& b) { return !operator==(a, b); }

	/** True if it's safe to execute this delegate, and it will execute a bound functor */
	inline operator bool() const { return IsValid() && IsBound(); }
	/** Executes the delegate with the provided parameters */
	inline ReturnType operator()(ParamTypes... params) const { return executor(context, storage, std::forward<ParamTypes>(params)...); }

	/** Create using a free function */
	static TDelegate Create(ReturnType(*function)(ParamTypes...));

	/** Create using a free function called with a raw context object */
	template<typename ObjectType>
	static TDelegate Create(ObjectType* instance, ReturnType(*function)(ObjectType*, ParamTypes...));
	/** Create using a free function called with a smart context object */
	template<typename ObjectType>
	static TDelegate Create(std::shared_ptr<ObjectType>&& instance, ReturnType(*function)(ObjectType*, ParamTypes...));

	/** Create using a method called on a raw context object */
	template<typename ObjectType>
	static TDelegate Create(ObjectType* instance, ReturnType(ObjectType::* method)(ParamTypes...));
	/** Create using a method called on a smart context object */
	template<typename ObjectType>
	static TDelegate Create(std::shared_ptr<ObjectType>&& instance, ReturnType(ObjectType::* method)(ParamTypes...));

	/** Create using a lambda */
	template<typename LambdaType>
	static TDelegate Create(LambdaType&& lambda);
	/** Create using a lambda called with a raw context object */
	template<typename ObjectType, typename LambdaType>
	static TDelegate Create(ObjectType* instance, LambdaType&& lambda);
	/** Create using a lambda called with a smart context object */
	template<typename ObjectType, typename LambdaType>
	static TDelegate Create(std::shared_ptr<ObjectType>&& instance, LambdaType&& lambda);

	/** Returns true if this delegate is actually bound to a functor that will be executed */
	inline bool IsBound() const {
		//@todo Implement this method
		return true;
	}

	/** Returns true if this delegate is bound to a method on the provided instance */
	template<typename ObjectType>
	inline bool IsBoundTo(ObjectType* instance) const { return semantics->IsBoundTo(instance); }
	/** Returns true if this delegate is bound to a method on the provided instance */
	template<typename ObjectType>
	inline bool IsBoundTo(std::shared_ptr<ObjectType> const& instance) const { return semantics->IsBoundTo(instance.get()); }

	/** Returns true if the context for this delegate is valid. Only meaningful for delegates where the context allows validity checking. */
	inline bool IsValid() const { return semantics->IsValid(*this); }
	/** Unbind the delegate so that it no longer executes any specific behavior, essentially returning it to a default-constructed state. */
	inline void Reset() { *this = TDelegate{}; }

};

/** a delegate which has no parameters and returns nothing */
using SimpleDelegate = TDelegate<void>;

//=======================================================================================
// Semantics implementations

/** Generic context semantics */
template<typename ActualContextType>
struct DelegateBase::TContextSemanticsImpl {
	static inline ActualContextType& Cast(DelegateContext& target) {
		if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::rawPointer)>) return target.rawPointer;
		else if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::constRawPointer)>) return target.constRawPointer;
		else if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::smartPointer)>) return target.smartPointer;
		else if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::constSmartPointer)>) return target.constSmartPointer;
		else {}
	}
	static inline ActualContextType const& Cast(const DelegateContext& target) {
		if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::rawPointer)>) return target.rawPointer;
		else if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::constRawPointer)>) return target.constRawPointer;
		else if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::smartPointer)>) return target.smartPointer;
		else if constexpr (std::is_convertible_v<ActualContextType, decltype(DelegateContext::constSmartPointer)>) return target.constSmartPointer;
		else {}
	}

	static constexpr bool IsRawPointerType = std::is_pointer_v<ActualContextType>;

	static inline void Construct(DelegateContext& target, ActualContextType const& value) noexcept {
		if constexpr (std::is_trivially_constructible_v<ActualContextType>) std::memcpy(&target, &value, sizeof(ActualContextType));
		else new (&Cast(target)) ActualContextType(value);
	}
	static inline void Construct(DelegateContext& target, ActualContextType&& value) noexcept {
		if constexpr (std::is_trivially_constructible_v<ActualContextType>) std::memcpy(&target, &value, sizeof(ActualContextType));
		else new (&Cast(target)) ActualContextType(std::move(value));
	}
	static inline void CopyConstruct(DelegateContext& target, DelegateContext const& source) noexcept { Construct(target, Cast(source)); }
	static inline void MoveConstruct(DelegateContext& target, DelegateContext&& source) noexcept { Construct(target, std::move(Cast(source))); }

	static inline void Destruct(DelegateContext& target) noexcept {
		if constexpr (std::is_trivially_destructible_v<ActualContextType>) {}
		else Cast(target).~ActualContextType();
	}
	static inline bool IsValid(DelegateContext const& target) noexcept {
		if constexpr (IsRawPointerType) return true;
		else !Cast(target).expired();
	}
	static inline bool IsBoundTo(DelegateContext const& target, void const* object) noexcept {
		if constexpr (IsRawPointerType) return Cast(target) == object;
		else return Cast(target).lock().get() == object;
	}
	static inline bool IsContextEqual(DelegateContext const& target, DelegateContext const& other) {
		if constexpr (IsRawPointerType) return Cast(target) == Cast(other);
		else return Cast(target).lock() == Cast(other).lock();
	}
};
/** No context semantics (used for static functions and empty delegates) */
template<>
struct DelegateBase::TContextSemanticsImpl<void> {
	static constexpr bool IsRawPointerType = true;
	static inline void CopyConstruct(DelegateContext& target, DelegateContext const& source) noexcept {}
	static inline void MoveConstruct(DelegateContext& target, DelegateContext&& source) noexcept {}
	static inline void Destruct(DelegateContext& target) noexcept {}
	static inline bool IsValid(DelegateContext const& target) noexcept { return true; }
	static inline bool IsBoundTo(DelegateContext const& target, void const* object) noexcept { return false; }
	static inline bool IsContextEqual(DelegateContext const& target, DelegateContext const& other) { return true; }
};

/** Generic storage semantics */
template<typename ActualStorageType>
struct DelegateBase::TStorageSemanticsImpl {
	static inline ActualStorageType& Get(DelegateStorage& storage) {
		if constexpr (IsInternal<ActualStorageType>()) return *reinterpret_cast<ActualStorageType*>(std::addressof(storage.internal));
		else return *static_cast<ActualStorageType*>(storage.external);
	}
	static inline ActualStorageType const& Get(DelegateStorage const& storage) {
		if constexpr (IsInternal<ActualStorageType>()) return *reinterpret_cast<ActualStorageType const*>(std::addressof(storage.internal));
		else return *static_cast<ActualStorageType const*>(storage.external);
	}

	static inline void Construct(DelegateStorage& target, ActualStorageType&& value) noexcept {
		if constexpr (IsInternal<ActualStorageType>()) new (&Get(target)) ActualStorageType(std::move(value));
		else target.external = new ActualStorageType(std::move(value));
	}
	static inline void Construct(DelegateStorage& target, ActualStorageType const& value) noexcept {
		if constexpr (IsInternal<ActualStorageType>()) new (&Get(target)) ActualStorageType(value);
		else target.external = new ActualStorageType(value);
	}

	static inline void CopyConstruct(DelegateStorage& target, DelegateStorage const& source) noexcept {
		Construct(target, Get(source));
	}
	static inline void MoveConstruct(DelegateStorage& target, DelegateStorage&& source) noexcept {
		//@todo Maybe rethink this. For external storage, it may be better if the source delegate is left in a
		//      nulled state and we just capture the storage pointer from it. The downside is that all other
		//      methods need to check if the pointer is null.
		Construct(target, std::move(Get(source)));
	}
	static inline void Destruct(DelegateStorage& target) noexcept {
		if (IsInternal<ActualStorageType>()) Get(target).~ActualStorageType();
		else delete& Get(target);
	}

	static inline bool IsStorageEqual(DelegateStorage const& target, DelegateStorage const& other) noexcept { return Get(target) == Get(other); }
};
/** No storage semantics (used for empty delegates) */
template<>
struct DelegateBase::TStorageSemanticsImpl<void> {
	static inline void CopyConstruct(DelegateStorage& target, DelegateStorage const& source) noexcept {}
	static inline void MoveConstruct(DelegateStorage& target, DelegateStorage&& source) noexcept {}
	static inline void Destruct(DelegateStorage& target) noexcept {}
	static inline bool IsStorageEqual(DelegateStorage const& target, DelegateStorage const& other) noexcept { return true; }
};

//=======================================================================================
// Delegate method implementations

/** Free function */
template<typename ReturnType, typename... ParamTypes>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(ReturnType(*function)(ParamTypes...)) {
	using ContextSemanticsImplType = TContextSemanticsImpl<void>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(function)>;

	TDelegate newDelegate;
	newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
	newDelegate.executor = [](DelegateContext const&, DelegateStorage const& storage, ParamTypes... params) {
		return StorageSemanticsImplType::Get(storage)(std::forward<ParamTypes>(params)...);
	};
	StorageSemanticsImplType::Construct(newDelegate.storage, function);
	return newDelegate;
}

/** Free functino and raw pointer */
template<typename ReturnType, typename... ParamTypes>
template<typename ObjectType>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(ObjectType* instance, ReturnType(*function)(ObjectType*, ParamTypes...)) {
	using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional_t<std::is_const_v<ObjectType>, void const*, void*>>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(function)>;

	TDelegate newDelegate;
	newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
	newDelegate.executor = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
		return StorageSemanticsImplType::Get(storage)(ContextSemanticsImplType::Get(context), std::forward<ParamTypes>(params)...);
	};

	ContextSemanticsImplType::Construct(newDelegate.context, instance);
	StorageSemanticsImplType::Construct(newDelegate.storage, function);
	return newDelegate;
}

/** Free function and smart pointer */
template<typename ReturnType, typename... ParamTypes>
template<typename ObjectType>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(std::shared_ptr<ObjectType>&& instance, ReturnType(*function)(ObjectType*, ParamTypes...)) {
	using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional_t<std::is_const_v<ObjectType>, std::weak_ptr<void const>, std::weak_ptr<void>>>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(function)>;

	TDelegate newDelegate;
	newDelegate.semantics.Construct<TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>>();
	newDelegate.executor = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
		if (const std::shared_ptr<ObjectType> lockedContext = StorageSemanticsImplType::Get(storage).lock()) {
			return StorageSemanticsImplType::Get(storage)(lockedContext.get(), std::forward<ParamTypes>(params)...);
		}
		else {
			return ReturnType{};
		}
	};

	ContextSemanticsImplType::Construct(newDelegate.context, std::forward<std::weak_ptr<ObjectType>>(instance));
	StorageSemanticsImplType::Construct(newDelegate.storage, function);
	return newDelegate;
}

/** Method and raw pointer */
template<typename ReturnType, typename... ParamTypes>
template<typename ObjectType>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(ObjectType* instance, ReturnType(ObjectType::* method)(ParamTypes...)) {
	using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional_t<std::is_const_v<ObjectType>, void const*, void*>>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(method)>;

	TDelegate newDelegate;
	newDelegate.semantics.Construct<TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>>();
	newDelegate.executor = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
		return (static_cast<ObjectType*>(ContextSemanticsImplType::Cast(context))->*StorageSemanticsImplType::Get(storage))(std::forward<ParamTypes>(params)...);
	};

	ContextSemanticsImplType::Construct(newDelegate.context, instance);
	StorageSemanticsImplType::Construct(newDelegate.storage, method);
	return newDelegate;
}

/** Method and smart pointer */
template<typename ReturnType, typename... ParamTypes>
template<typename ObjectType>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(std::shared_ptr<ObjectType>&& instance, ReturnType(ObjectType::* method)(ParamTypes...)) {
	using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional_t<std::is_const_v<ObjectType>, std::weak_ptr<void const>, std::weak_ptr<void>>>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(method)>;

	TDelegate newDelegate;
	newDelegate.semantics.Construct<TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>>();
	newDelegate.executor = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
		if (const std::shared_ptr<ObjectType> lockedContext = ContextSemanticsImplType::Cast(context).lock()) {
			return (static_cast<ObjectType*>(lockedContext.get())->*StorageSemanticsImplType::Get(storage))(std::forward<ParamTypes>(params)...);
		}
		else {
			return ReturnType{};
		}
	};

	ContextSemanticsImplType::Construct(newDelegate.context, instance);
	StorageSemanticsImplType::Construct(newDelegate.storage, method);
	return newDelegate;
}

/** Lambda */
template<typename ReturnType, typename... ParamTypes>
template<typename LambdaType>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(LambdaType&& lambda) {
	using ContextSemanticsImplType = TContextSemanticsImpl<void>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<typename std::decay_t<decltype(lambda)>>;

	TDelegate newDelegate;
	newDelegate.semantics.Construct<TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>>();
	newDelegate.executor = [](DelegateContext const&, DelegateStorage const& storage, ParamTypes... params) {
		return StorageSemanticsImplType::Get(storage)(std::forward<ParamTypes>(params)...);
	};

	StorageSemanticsImplType::Construct(newDelegate.storage, std::forward<LambdaType>(lambda));
	return newDelegate;
}

/** Lambda and raw pointer */
template<typename ReturnType, typename... ParamTypes>
template<typename ObjectType, typename LambdaType>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(ObjectType* instance, LambdaType&& lambda) {
	using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional_t<std::is_const_v<ObjectType>, void const*, void*>>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<typename std::decay_t<decltype(lambda)>>;

	TDelegate newDelegate;
	newDelegate.semantics.Construct<TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>>();
	newDelegate.executor = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
		return StorageSemanticsImplType::Get(storage)(ContextSemanticsImplType::Cast(context), std::forward<ParamTypes>(params)...);
	};

	ContextSemanticsImplType::Construct(newDelegate.context, instance);
	StorageSemanticsImplType::Construct(newDelegate.storage, std::forward<LambdaType>(lambda));
	return newDelegate;
}

/** Lambda and smart pointer */
template<typename ReturnType, typename... ParamTypes>
template<typename ObjectType, typename LambdaType>
TDelegate<ReturnType, ParamTypes...> TDelegate<ReturnType, ParamTypes...>::Create(std::shared_ptr<ObjectType>&& instance, LambdaType&& lambda) {
	using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional_t<std::is_const_v<ObjectType>, std::weak_ptr<void const>, std::weak_ptr<void>>>;
	using StorageSemanticsImplType = TStorageSemanticsImpl<typename std::decay_t<decltype(lambda)>>;

	TDelegate newDelegate;
	newDelegate.semantics.Construct<TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>>();
	newDelegate.executor = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
		if (const std::shared_ptr<ObjectType> lockedContext = ContextSemanticsImplType::Cast(context).lock()) {
			return StorageSemanticsImplType::Get(storage)(lockedContext.get(), std::forward<ParamTypes>(params)...);
		}
		else {
			return ReturnType{};
		}
	};

	ContextSemanticsImplType::Construct(newDelegate.context, std::forward<std::weak_ptr<ObjectType>>(instance));
	StorageSemanticsImplType::Construct(newDelegate.storage, std::forward<LambdaType>(lambda));
	return newDelegate;
}
