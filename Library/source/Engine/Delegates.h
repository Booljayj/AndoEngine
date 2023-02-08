#pragma once
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

struct DelegateBase {
protected:
	/** The type used for the delegate's context object, if one exists */
	union DelegateContext {
		void* rawPointer;
		std::weak_ptr<void> smartPointer;
		void const* constRawPointer;
		std::weak_ptr<void const> constSmartPointer;

		//Types are managed by the semantics objects, so this union is left uninitialized
		DelegateContext() {}
		~DelegateContext() {}
	};
	/** The type used to store the actual callable object wrapped by this delegate. */
	union DelegateStorage {
		void* external;
		alignas(PointerTraits::FunctionPointerAlign) std::byte internal[PointerTraits::FunctionPointerSize];

		//Types are managed by the semantics objects, so this union is left uninitialized
		DelegateStorage() {}
		~DelegateStorage() {}
	};

	//Helper template to determine if a type can fit in internal storage
	template<typename ActualStorageType>
	static constexpr bool IsInternal() { return sizeof(ActualStorageType) <= sizeof(DelegateStorage::internal); }

	//Templates that are specialized with semantics for various types, to ensure proper behavior when constructing, destructing, and comparing them.
	template<typename ActualContextType>
	struct TContextSemanticsImpl;
	template<typename ActualStorageType>
	struct TStorageSemanticsImpl;

	//Base interface for semantics, which erases type information but handles basic operations
	struct IDelegateSemantics {
		virtual void CopyConstruct(DelegateBase& target, const DelegateBase& source) const noexcept = 0;
		virtual void MoveConstruct(DelegateBase& target, DelegateBase&& source) const noexcept = 0;
		virtual void Destruct(DelegateBase& target) const noexcept = 0;
		virtual bool IsValid(DelegateBase const& target) const noexcept = 0;
	};

	//Wraps the semantics for the context and storage into the generic IDelegateSemantics interface
	template<typename ContextSemanticsImplType, typename StorageSemanticsImplType>
	struct TWrappedSemantics : public IDelegateSemantics {
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
	};

	//Create a static instance of wrapped semantics, to ensure that the same combination of context and
	//storage types is re-used among different delegate types and to ensure proper initialization order.
	template<typename ContextSemanticsImplType, typename StorageSemanticsImplType>
	static TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType> const* StaticCreateWrappedSemantics() {
		static TWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType> wrappedSemantics{};
		return &wrappedSemantics;
	}

	IDelegateSemantics const* semantics;
	DelegateContext context;
	DelegateStorage storage;

	DelegateBase() = default; //Protected constructor ensures this cannot be instantiated directly

	friend bool operator==(DelegateBase const& a, DelegateBase const& b);
	friend bool operator!=(DelegateBase const& a, DelegateBase const& b);

public:
	virtual ~DelegateBase() {}
};

inline bool operator==(DelegateBase const& a, DelegateBase const& b) {
	if (a.semantics != b.semantics) {
		return false;
	} else {
		//@todo Implement semantic equality testing
		return true;
	}
}
inline bool operator!=(DelegateBase const& a, DelegateBase const& b) { return !operator==(a, b); }

/** Raw pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<void*> {
	static inline void* Get(const DelegateContext& target) { return target.rawPointer; }
	static inline void Construct(DelegateContext& target, void* value) noexcept {target.rawPointer = value;}
	static inline void CopyConstruct(DelegateContext& target, DelegateContext const& source) noexcept {target.rawPointer = source.rawPointer;}
	static inline void MoveConstruct(DelegateContext& target, DelegateContext&& source) noexcept {target.rawPointer = source.rawPointer;}
	static inline void Destruct(DelegateContext& target) noexcept {}
	static inline bool IsValid(DelegateContext const& target) noexcept { return true; }
};
/** Const raw pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<void const*> {
	static inline void const* Get(const DelegateContext& target) { return target.constRawPointer; }
	static inline void Construct(DelegateContext& target, void const* value) noexcept {target.constRawPointer = value;}
	static inline void CopyConstruct(DelegateContext& target, DelegateContext const& source) noexcept {target.constRawPointer = source.constRawPointer;}
	static inline void MoveConstruct(DelegateContext& target, DelegateContext&& source) noexcept {target.constRawPointer = source.constRawPointer;}
	static inline void Destruct(DelegateContext& target) noexcept {}
	static inline bool IsValid(DelegateContext const& target) noexcept { return true; }
};
/** Smart pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<std::weak_ptr<void>> {
	static inline std::weak_ptr<void>& Get(DelegateContext& target) { return target.smartPointer; }
	static inline std::weak_ptr<void> const& Get(DelegateContext const& target) { return target.smartPointer; }

	static inline void Construct(DelegateContext& target, std::weak_ptr<void> const& value) noexcept {
		new (&target.smartPointer) std::weak_ptr<void>(value);
	}
	static inline void Construct(DelegateContext& target, std::weak_ptr<void>&& value) noexcept {
		new (&target.smartPointer) std::weak_ptr<void>(std::move(value));
	}

	static inline void CopyConstruct(DelegateContext& target, DelegateContext const& source) noexcept { Construct(target, Get(source)); }
	static inline void MoveConstruct(DelegateContext& target, DelegateContext&& source) noexcept { Construct(target, std::move(Get(source))); }
	static inline void Destruct(DelegateContext& target) noexcept { Get(target).~weak_ptr<void>(); }
	static inline bool IsValid(DelegateContext& target) noexcept { return !Get(target).expired(); }
};
/** Const smart pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<std::weak_ptr<void const>> {
	static inline std::weak_ptr<void const>& Get(DelegateContext& target) { return target.constSmartPointer; }
	static inline std::weak_ptr<void const> const& Get(DelegateContext const& target) { return target.constSmartPointer; }

	static inline void Construct(DelegateContext& target, std::weak_ptr<void const> const& value) noexcept {
		new (&target.constSmartPointer) std::weak_ptr<void const>(value);
	}
	static inline void Construct(DelegateContext& target, std::weak_ptr<void>&& value) noexcept {
		new (&target.constSmartPointer) std::weak_ptr<void const>(std::move(value));
	}

	static inline void CopyConstruct(DelegateContext& target, DelegateContext const& source) noexcept { Construct(target, Get(source)); }
	static inline void MoveConstruct(DelegateContext& target, DelegateContext&& source) noexcept { Construct(target, std::move(Get(source))); }
	static inline void Destruct(DelegateContext& target) noexcept { Get(target).~weak_ptr<void const>(); }
	static inline bool IsValid(DelegateContext const& target) noexcept { return !Get(target).expired(); }
};
/** No context (used for static functions and empty delegates) */
template<>
struct DelegateBase::TContextSemanticsImpl<void> {
	static inline void CopyConstruct(DelegateContext& target, DelegateContext const& source) noexcept {}
	static inline void MoveConstruct(DelegateContext& target, DelegateContext&& source) noexcept {}
	static inline void Destruct(DelegateContext& target) noexcept {}
	static inline bool IsValid(DelegateContext const& target) noexcept { return true; }
};

/** Standard storage */
template<typename ActualStorageType>
struct DelegateBase::TStorageSemanticsImpl {
	static inline ActualStorageType& Get(DelegateStorage& storage) {
		if (IsInternal<ActualStorageType>())
			return *reinterpret_cast<ActualStorageType*>(std::addressof(storage.internal));
		else
			return *static_cast<ActualStorageType*>(storage.external);
	}
	static inline ActualStorageType const& Get(DelegateStorage const& storage) {
		if (IsInternal<ActualStorageType>())
			return *reinterpret_cast<ActualStorageType const*>(std::addressof(storage.internal));
		else
			return *static_cast<ActualStorageType const*>(storage.external);
	}

	static inline void Construct(DelegateStorage& target, ActualStorageType&& value) noexcept {
		if (IsInternal<ActualStorageType>())
			new (&Get(target)) ActualStorageType(std::move(value));
		else
			target.external = new ActualStorageType(std::move(value));
	}
	static inline void Construct(DelegateStorage& target, ActualStorageType const& value) noexcept {
		if (IsInternal<ActualStorageType>())
			new (&Get(target)) ActualStorageType(value);
		else
			target.external = new ActualStorageType(value);
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
		if (IsInternal<ActualStorageType>())
			Get(target).~ActualStorageType();
		else
			delete &Get(target);
	}
};
/** No storage (used for empty delegates) */
template<>
struct DelegateBase::TStorageSemanticsImpl<void> {
	static inline void CopyConstruct(DelegateStorage& target, DelegateStorage const& source) noexcept {}
	static inline void MoveConstruct(DelegateStorage& target, DelegateStorage&& source) noexcept {}
	static inline void Destruct(DelegateStorage& target) noexcept {}
};

template<typename ReturnType, typename... ParamTypes>
struct TDelegate : public DelegateBase {
private:
	/** Stub function used to invoke the bound operation with the correct semantics using a generic interface */
	using StubFunctionType = ReturnType(*)(DelegateContext const&, DelegateStorage const&, ParamTypes...);
	StubFunctionType stubFunction;

public:
	TDelegate() {
		semantics = StaticCreateWrappedSemantics<TContextSemanticsImpl<void>, TStorageSemanticsImpl<void>>();
		stubFunction = [](DelegateContext const&, DelegateStorage const&, ParamTypes...) {};
	}
	~TDelegate() {
		semantics->Destruct(*this);
		//Ensures that double-destruction and invoking a destructed delegate will segfault.
		semantics = nullptr;
		stubFunction = nullptr;
	}

	TDelegate(TDelegate const& other) : TDelegate() {
		semantics->Destruct(*this);
		semantics = other.semantics;
		stubFunction = other.stubFunction;
		semantics->CopyConstruct(*this, other);
	}
	TDelegate(TDelegate&& other) : TDelegate() {
		semantics->Destruct(*this);
		semantics = other.semantics;
		stubFunction = other.stubFunction;
		semantics->MoveConstruct(*this, std::move(other));
	}
	TDelegate& operator=(TDelegate const& other) {
		semantics->Destruct(*this);
		semantics = other.semantics;
		stubFunction = other.stubFunction;
		semantics->CopyConstruct(*this, other);
		return *this;
	}
	TDelegate& operator=(TDelegate&& other) {
		semantics->Destruct(*this);
		semantics = other.semantics;
		stubFunction = other.stubFunction;
		semantics->MoveConstruct(*this, std::move(other));
		return *this;
	}

	/** True if it's safe to execute this delegate, and it will execute a bound functor */
	inline operator bool() const {
		return IsValid() && IsBound();
	}
	/** Executes the delegate with the provided parameters */
	inline ReturnType operator()(ParamTypes... params) const {
		return stubFunction(context, storage, std::forward<ParamTypes>(params)...);
	}

	/** Returns true if this delegate is actually bound to a functor that will be executed */
	inline bool IsBound() const {
		//We know this delegate is bound if it's not using the special null semantics, since there is nothing in the context or storage.
		return semantics != StaticCreateWrappedSemantics<TContextSemanticsImpl<void>, TStorageSemanticsImpl<void>>();
	}
	/** Returns true if the context for this delegate is valid. Only meaningful for delegates where the context allows validity checking. */
	inline bool IsValid() const {
		return semantics->IsValid(*this);
	}
	/** Unbind the delegate so that it no longer executes any specific behavior, essentially returning it to a default-constructed state. */
	inline void Reset() {
		*this = TDelegate{};
	}

	/** Create using a free function */
	static TDelegate Create(ReturnType(*function)(ParamTypes...)) {
		using ContextSemanticsImplType = TContextSemanticsImpl<void>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(function)>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const&, DelegateStorage const& storage, ParamTypes... params) {
			return StorageSemanticsImplType::Get(storage)(std::forward<ParamTypes>(params)...);
		};
		StorageSemanticsImplType::Construct(newDelegate.storage, function);
		return newDelegate;
	}
	/** Create using a free function called with a raw context object */
	template<typename ObjectType>
	static TDelegate Create(ObjectType* instance, ReturnType(*function)(ObjectType*, ParamTypes...)) {
		using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional<std::is_const<ObjectType>::value, void const*, void*>::type>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(function)>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
			return StorageSemanticsImplType::Get(storage)(ContextSemanticsImplType::Get(context), std::forward<ParamTypes>(params)...);
		};

		ContextSemanticsImplType::Construct(newDelegate.context, instance);
		StorageSemanticsImplType::Construct(newDelegate.storage, function);
		return newDelegate;
	}
	/** Create using a free function called with a smart context object */
	template<typename ObjectType>
	static TDelegate Create(std::weak_ptr<ObjectType>&& instance, ReturnType(*function)(ObjectType*, ParamTypes...)) {
		using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional<std::is_const<ObjectType>::value, std::weak_ptr<void const>, std::weak_ptr<void>>::type>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(function)>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
			if(const std::shared_ptr<ObjectType> lockedContext = StorageSemanticsImplType::Get(storage).lock()) {
				return StorageSemanticsImplType::Get(storage)(lockedContext.get(), std::forward<ParamTypes>(params)...);
			} else {
				return ReturnType{};
			}
		};

		ContextSemanticsImplType::Construct(newDelegate.context, std::forward<std::weak_ptr<ObjectType>>(instance));
		StorageSemanticsImplType::Construct(newDelegate.storage, function);
		return newDelegate;
	}

	/** Create using a method called on a raw context object */
	template<typename ObjectType>
	static TDelegate Create(ObjectType* instance, ReturnType(ObjectType::*method)(ParamTypes...)) {
		using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional<std::is_const<ObjectType>::value, void const*, void*>::type>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(method)>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
			return (static_cast<ObjectType*>(ContextSemanticsImplType::Get(context))->*StorageSemanticsImplType::Get(storage))(std::forward<ParamTypes>(params)...);
		};

		ContextSemanticsImplType::Construct(newDelegate.context, instance);
		StorageSemanticsImplType::Construct(newDelegate.storage, method);
		return newDelegate;
	}
	/** Create using a method called on a smart context object */
	template<typename ObjectType>
	static TDelegate Create(std::weak_ptr<ObjectType>&& instance, ReturnType(ObjectType::*method)(ParamTypes...)) {
		using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional<std::is_const<ObjectType>::value, std::weak_ptr<void const>, std::weak_ptr<void>>::type>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<decltype(method)>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
			if( const std::shared_ptr<ObjectType> lockedContext = ContextSemanticsImplType::Get(context).lock() ) {
				return (static_cast<ObjectType*>(lockedContext.get())->*StorageSemanticsImplType::Get(storage))(std::forward<ParamTypes>(params)...);
			} else {
				return ReturnType{};
			}
		};

		ContextSemanticsImplType::Construct(newDelegate.context, instance);
		StorageSemanticsImplType::Construct(newDelegate.storage, method);
		return newDelegate;
	}

	/** Create using a lambda */
	template<typename LambdaType>
	static TDelegate Create(LambdaType&& lambda) {
		using ContextSemanticsImplType = TContextSemanticsImpl<void>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<typename std::decay<decltype(lambda)>::type>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const&, DelegateStorage const& storage, ParamTypes... params) {
			return StorageSemanticsImplType::Get(storage)(std::forward<ParamTypes>(params)...);
		};

		StorageSemanticsImplType::Construct(newDelegate.storage, std::forward<LambdaType>(lambda));
		return newDelegate;
	}
	/** Create using a lambda called with a raw context object */
	template<typename ObjectType, typename LambdaType>
	static TDelegate Create(ObjectType* instance, LambdaType&& lambda) {
		using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional<std::is_const<ObjectType>::value, void const*, void*>::type>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<typename std::decay<decltype(lambda)>::type>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
			return StorageSemanticsImplType::Get(storage)(ContextSemanticsImplType::Get(context), std::forward<ParamTypes>(params)...);
		};

		ContextSemanticsImplType::Construct(newDelegate.context, instance);
		StorageSemanticsImplType::Construct(newDelegate.storage, std::forward<LambdaType>(lambda));
		return newDelegate;
	}
		/** Create using a lambda called with a smart context object */
	template<typename ObjectType, typename LambdaType>
	static TDelegate Create(std::weak_ptr<ObjectType>&& instance, LambdaType&& lambda) {
		using ContextSemanticsImplType = TContextSemanticsImpl<typename std::conditional<std::is_const<ObjectType>::value, std::weak_ptr<void const>, std::weak_ptr<void>>::type>;
		using StorageSemanticsImplType = TStorageSemanticsImpl<typename std::decay<decltype(lambda)>::type>;

		TDelegate newDelegate;
		newDelegate.semantics = StaticCreateWrappedSemantics<ContextSemanticsImplType, StorageSemanticsImplType>();
		newDelegate.stubFunction = [](DelegateContext const& context, DelegateStorage const& storage, ParamTypes... params) {
			if( const std::shared_ptr<ObjectType> lockedContext = ContextSemanticsImplType::Get(context).lock()) {
				return StorageSemanticsImplType::Get(storage)(lockedContext.get(), std::forward<ParamTypes>(params)...);
			} else {
				return ReturnType{};
			}
		};

		ContextSemanticsImplType::Construct(newDelegate.context, std::forward<std::weak_ptr<ObjectType>>(instance));
		StorageSemanticsImplType::Construct(newDelegate.storage, std::forward<LambdaType>(lambda));
		return newDelegate;
	}
};

/** a delegate which has no parameters and returns nothing */
using SimpleDelegate = TDelegate<void>;
