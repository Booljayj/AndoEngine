#pragma once
#include <type_traits>
#include <memory>

//Delegates wrap a callable together with an optional context object. When executed, the callable
//and context object are used together to invoke the specified behavior. Based on "The Impossibly
//Fast C++ Delegates" by Sergey Ryazanov and similar resources, but heavily modified to support
//a larger feature set and reduce allocations.
//https://www.codeproject.com/articles/11015/the-impossibly-fast-c-delegates

//Delegates are defined using the signature of the callable, and are created using static methods
//on that specialization. For example:
//- Delegate has signature void(), and is bound to a free function that has that signature
//		Delagate<void> D = Delegate<void>::Create( &FreeFunction );
//- Delegate has signature int(std::string), and is bound to a method that has that signature
//		ClassType Instance;
//		Delegate<int, std::string> D = Delegate<int, std::string>::Create( &Instance, &ClassType::Method );
//- Delegate has signature void(char), and is bound to a lambda that also accepts the context object
//		ClassType Instance;
//		Delegate<void, char> D = Delegate<void, char>::Create( &Instance, [](ClassType* I){ /*...*/ } );

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
	//DummyClassMethod should be the worst-case-scenario for the size of a method pointer. Because the class is only
	//forward declared, the compiler cannot make any assumptions about the inheritance structure, and must use a
	//method pointer implementation that can handle the most complex case (typ virtual inheritance). Note: on some
	//compilers, aligned_storage will round up its size to the next aligned value. This means storage is a little
	//larger than strictly necessary, but that extra space can still be used for lambdas.
	class DummyClass;
	using DummyClassMethod = void(DummyClass::*)();

	/** The type used for the delegate's context object, if one exists */
	union ContextType {
		void* RawPointer;
		std::weak_ptr<void> SmartPointer;
		void const* ConstRawPointer;
		std::weak_ptr<void const> ConstSmartPointer;

		//Types are managed by the semantics objects, so this union is left uninitialized
		ContextType() {}
		~ContextType() {}
	};
	/** The type used to store the actual callable object wrapped by this delegate. */
	union StorageType {
		void* External;
		std::aligned_storage<sizeof(DummyClassMethod), alignof(void*)>::type Internal;

		//Types are managed by the semantics objects, so this union is left uninitialized
		StorageType() {}
		~StorageType() {}
	};

	//Helper template to determine if a type can fit in internal storage
	template<typename TSTORAGE>
	static constexpr bool IsInternal() { return sizeof(TSTORAGE) <= sizeof(StorageType::Internal); }

	//Templates that are specialized with semantics for various types, to ensure proper behavior when constructing, destructing, and comparing them.
	template<typename TCONTEXT>
	struct TContextSemanticsImpl;
	template<typename TSTORAGE>
	struct TStorageSemanticsImpl;

	//Base interface for semantics, which erases type information but handles basic operations
	struct IDelegateSemantics {
		virtual void CopyConstruct(DelegateBase& Target, const DelegateBase& Source) const noexcept = 0;
		virtual void MoveConstruct(DelegateBase& Target, DelegateBase&& Source) const noexcept = 0;
		virtual void Destruct(DelegateBase& Target) const noexcept = 0;
	};

	//Wraps the semantics for the context and storage into the generic IDelegateSemantics interface
	template<typename ContextSemanticsImpl, typename StorageSemanticsImpl>
	struct TWrappedSemantics : public IDelegateSemantics {
		virtual void CopyConstruct(DelegateBase& Target, const DelegateBase& Source) const noexcept override {
			ContextSemanticsImpl::CopyConstruct(Target.Context, Source.Context);
			StorageSemanticsImpl::CopyConstruct(Target.Storage, Source.Storage);
		}
		virtual void MoveConstruct(DelegateBase& Target, DelegateBase&& Source) const noexcept override {
			ContextSemanticsImpl::MoveConstruct(Target.Context, std::move(Source.Context));
			StorageSemanticsImpl::MoveConstruct(Target.Storage, std::move(Source.Storage));
		}
		virtual void Destruct(DelegateBase& Target) const noexcept override {
			ContextSemanticsImpl::Destruct(Target.Context);
			StorageSemanticsImpl::Destruct(Target.Storage);
		}
	};

	//Create a static instance of wrapped semantics, to ensure that the same combination of context and
	//storage types is re-used among different delegate types and to ensure proper initialization order.
	template<typename ContextSemanticsImpl, typename StorageSemanticsImpl>
	static TWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl> const* StaticCreateWrappedSemantics() {
		static TWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl> WrappedSemantics{};
		return &WrappedSemantics;
	}

	IDelegateSemantics const* Semantics;
	ContextType Context;
	StorageType Storage;

	DelegateBase() = default; //Protected constructor ensures this cannot be instantiated directly

	friend bool operator==(DelegateBase const& A, DelegateBase const& B);
	friend bool operator!=(DelegateBase const& A, DelegateBase const& B);

public:
	virtual ~DelegateBase() {}
};

inline bool operator==(DelegateBase const& A, DelegateBase const& B) {
	if( A.Semantics != B.Semantics) {
		return false;
	} else {
		//@todo Implement semantic equality testing
		return true;
	}
}
inline bool operator!=(DelegateBase const& A, DelegateBase const& B) { return !operator==(A, B); }

/** Raw pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<void*> {
	static inline void* Get(const ContextType& Target) { return Target.RawPointer; }
	static inline void Construct(ContextType& Target, void* Value) noexcept {Target.RawPointer = Value;}
	static inline void CopyConstruct(ContextType& Target, ContextType const& Source) noexcept {Target.RawPointer = Source.RawPointer;}
	static inline void MoveConstruct(ContextType& Target, ContextType&& Source) noexcept {Target.RawPointer = Source.RawPointer;}
	static inline void Destruct(ContextType& Target) noexcept {}
};
/** Const raw pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<void const*> {
	static inline void const* Get(const ContextType& Target) { return Target.ConstRawPointer; }
	static inline void Construct(ContextType& Target, void const* Value) noexcept {Target.ConstRawPointer = Value;}
	static inline void CopyConstruct(ContextType& Target, ContextType const& Source) noexcept {Target.ConstRawPointer = Source.ConstRawPointer;}
	static inline void MoveConstruct(ContextType& Target, ContextType&& Source) noexcept {Target.ConstRawPointer = Source.ConstRawPointer;}
	static inline void Destruct(ContextType& Target) noexcept {}
};
/** Smart pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<std::weak_ptr<void>> {
	static inline std::weak_ptr<void>& Get(ContextType& Target) { return Target.SmartPointer; }
	static inline std::weak_ptr<void> const& Get(ContextType const& Target) { return Target.SmartPointer; }

	static inline void Construct(ContextType& Target, std::weak_ptr<void> const& Value) noexcept {
		new (&Target.SmartPointer) std::weak_ptr<void>(Value);
	}
	static inline void Construct(ContextType& Target, std::weak_ptr<void>&& Value) noexcept {
		new (&Target.SmartPointer) std::weak_ptr<void>(std::move(Value));
	}

	static inline void CopyConstruct(ContextType& Target, ContextType const& Source) noexcept { Construct(Target, Get(Source)); }
	static inline void MoveConstruct(ContextType& Target, ContextType&& Source) noexcept { Construct(Target, std::move(Get(Source))); }
	static inline void Destruct(ContextType& Target) noexcept { Get(Target).~weak_ptr<void>(); }
};
/** Const smart pointer context */
template<>
struct DelegateBase::TContextSemanticsImpl<std::weak_ptr<void const>> {
	static inline std::weak_ptr<void const>& Get(ContextType& Target) { return Target.ConstSmartPointer; }
	static inline std::weak_ptr<void const> const& Get(ContextType const& Target) { return Target.ConstSmartPointer; }

	static inline void Construct(ContextType& Target, std::weak_ptr<void const> const& Value) noexcept {
		new (&Target.ConstSmartPointer) std::weak_ptr<void const>(Value);
	}
	static inline void Construct(ContextType& Target, std::weak_ptr<void>&& Value) noexcept {
		new (&Target.ConstSmartPointer) std::weak_ptr<void const>(std::move(Value));
	}

	static inline void CopyConstruct(ContextType& Target, ContextType const& Source) noexcept { Construct(Target, Get(Source)); }
	static inline void MoveConstruct(ContextType& Target, ContextType&& Source) noexcept { Construct(Target, std::move(Get(Source))); }
	static inline void Destruct(ContextType& Target) noexcept { Get(Target).~weak_ptr<void const>(); }
};
/** No context */
template<>
struct DelegateBase::TContextSemanticsImpl<void> {
	static inline void CopyConstruct(ContextType& Target, ContextType const& Source) noexcept {}
	static inline void MoveConstruct(ContextType& Target, ContextType&& Source) noexcept {}
	static inline void Destruct(ContextType& Target) noexcept {}
};

/** Standard storage */
template<typename TSTORAGE>
struct DelegateBase::TStorageSemanticsImpl {
	static inline TSTORAGE& Get(StorageType& S) {
		if( IsInternal<TSTORAGE>() )
			return *reinterpret_cast<TSTORAGE*>(std::addressof(S.Internal));
		else
			return *static_cast<TSTORAGE*>(S.External);
	}
	static inline TSTORAGE const& Get(StorageType const& S) {
		if( IsInternal<TSTORAGE>() )
			return *reinterpret_cast<TSTORAGE const*>(std::addressof(S.Internal));
		else
			return *static_cast<TSTORAGE const*>(S.External);
	}

	static inline void Construct(StorageType& Target, TSTORAGE&& Value) noexcept {
		if( IsInternal<TSTORAGE>() )
			new (&Get(Target)) TSTORAGE(std::move(Value));
		else
			Target.External = new TSTORAGE(std::move(Value));
	}
	static inline void Construct(StorageType& Target, TSTORAGE const& Value) noexcept {
		if( IsInternal<TSTORAGE>() )
			new (&Get(Target)) TSTORAGE(Value);
		else
			Target.External = new TSTORAGE(Value);
	}

	static inline void CopyConstruct(StorageType& Target, StorageType const& Source) noexcept {
		Construct(Target, Get(Source));
	}
	static inline void MoveConstruct(StorageType& Target, StorageType&& Source) noexcept {
		Construct(Target, std::move(Get(Source)));
	}
	static inline void Destruct(StorageType& Target) noexcept {
		if( IsInternal<TSTORAGE>() )
			Get(Target).~TSTORAGE();
		else
			delete &Get(Target);
	}
};
/** No storage */
template<>
struct DelegateBase::TStorageSemanticsImpl<void> {
	static inline void CopyConstruct(StorageType& Target, StorageType const& Source) noexcept {}
	static inline void MoveConstruct(StorageType& Target, StorageType&& Source) noexcept {}
	static inline void Destruct(StorageType& Target) noexcept {}
};

template<typename R, typename... PARAMS>
struct Delegate : public DelegateBase {
private:
	/** Stub function used to invoke the bound operation with the correct semantics using a generic interface */
	using StubFunctionType = R(*)(ContextType const&, StorageType const&, PARAMS...);
	StubFunctionType StubFunction;

public:
	Delegate() {
		Semantics = StaticCreateWrappedSemantics<TContextSemanticsImpl<void>, TStorageSemanticsImpl<void>>();
		StubFunction = [](ContextType const&, StorageType const&, PARAMS...) {};
	}
	~Delegate() {
		Semantics->Destruct(*this);
		//Ensures that double-destruction and invoking a destructed delegate will segfault.
		Semantics = nullptr;
		StubFunction = nullptr;
	}

	Delegate(Delegate const& Other) {
		Semantics->Destruct(*this);
		Semantics = Other.Semantics;
		StubFunction = Other.StubFunction;
		Semantics->CopyConstruct(*this, Other);
	}
	Delegate(Delegate&& Other) {
		Semantics->Destruct(*this);
		Semantics = Other.Semantics;
		StubFunction = Other.StubFunction;
		Semantics->MoveConstruct(*this, std::move(Other));
	}
	Delegate& operator=(Delegate const& Other) {
		Semantics->Destruct(*this);
		Semantics = Other.Semantics;
		StubFunction = Other.StubFunction;
		Semantics->CopyConstruct(*this, Other);
		return *this;
	}
	Delegate& operator=(Delegate&& Other) {
		Semantics->Destruct(*this);
		Semantics = Other.Semantics;
		StubFunction = Other.StubFunction;
		Semantics->MoveConstruct(*this, std::move(Other));
		return *this;
	}

	/** Create using a free function */
	static Delegate Create( R(*Function)(PARAMS...) ) {
		using ContextSemanticsImpl = TContextSemanticsImpl<void>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<decltype(Function)>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const&, StorageType const& Storage, PARAMS... Params) {
			return StorageSemanticsImpl::Get(Storage)(std::forward<PARAMS>(Params)...);
		};
		StorageSemanticsImpl::Construct(NewDelegate.Storage, Function);
		return NewDelegate;
	}
	/** Create using a free function called with a raw context object */
	template<typename C>
	static Delegate Create(C* Instance, R(*Function)(C*, PARAMS...)) {
		using ContextSemanticsImpl = TContextSemanticsImpl<typename std::conditional<std::is_const<C>::value, void const*, void*>::type>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<decltype(Function)>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const& Context, StorageType const& Storage, PARAMS... Params) {
			return StorageSemanticsImpl::Get(Storage)(ContextSemanticsImpl::Get(Context), std::forward<PARAMS>(Params)...);
		};

		ContextSemanticsImpl::Construct(NewDelegate.Context, Instance);
		StorageSemanticsImpl::Construct(NewDelegate.Storage, Function);
		return NewDelegate;
	}
	/** Create using a free function called with a smart context object */
	template<typename C>
	static Delegate Create(std::weak_ptr<C>&& Instance, R(*Function)(C*, PARAMS...)) {
		using ContextSemanticsImpl = TContextSemanticsImpl<typename std::conditional<std::is_const<C>::value, std::weak_ptr<void const>, std::weak_ptr<void>>::type>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<decltype(Function)>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const& Context, StorageType const& Storage, PARAMS... Params) {
			if(const std::shared_ptr<C> LockedContext = StorageSemanticsImpl::Get(Storage).lock()) {
				return StorageSemanticsImpl::Get(Storage)(LockedContext.get(), std::forward<PARAMS>(Params)...);
			} else {
				return R{};
			}
		};

		ContextSemanticsImpl::Construct(NewDelegate.Context, std::forward<std::weak_ptr<C>>(Instance));
		StorageSemanticsImpl::Construct(NewDelegate.Storage, Function);
		return NewDelegate;
	}

	/** Create using a method called on a raw context object */
	template<typename C>
	static Delegate Create(C* Instance, R(C::*Method)(PARAMS...)) {
		using ContextSemanticsImpl = TContextSemanticsImpl<typename std::conditional<std::is_const<C>::value, void const*, void*>::type>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<decltype(Method)>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const& Context, StorageType const& Storage, PARAMS... Params) {
			return (static_cast<C*>(ContextSemanticsImpl::Get(Context))->*StorageSemanticsImpl::Get(Storage))(std::forward<PARAMS>(Params)...);
		};

		ContextSemanticsImpl::Construct(NewDelegate.Context, Instance);
		StorageSemanticsImpl::Construct(NewDelegate.Storage, Method);
		return NewDelegate;
	}
	/** Create using a method called on a smart context object */
	template<typename C>
	static Delegate Create(std::weak_ptr<C>&& Instance, R(C::*Method)(PARAMS...)) {
		using ContextSemanticsImpl = TContextSemanticsImpl<typename std::conditional<std::is_const<C>::value, std::weak_ptr<void const>, std::weak_ptr<void>>::type>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<decltype(Method)>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const& Context, StorageType const& Storage, PARAMS... Params) {
			if( const std::shared_ptr<C> LockedContext = ContextSemanticsImpl::Get(Context).lock() ) {
				return (static_cast<C*>(LockedContext.get())->*StorageSemanticsImpl::Get(Storage))(std::forward<PARAMS>(Params)...);
			} else {
				return R{};
			}
		};

		ContextSemanticsImpl::Construct(NewDelegate.Context, Instance);
		StorageSemanticsImpl::Construct(NewDelegate.Storage, Method);
		return NewDelegate;
	}

	/** Create using a lambda */
	template<typename TLAMBDA>
	static Delegate Create(TLAMBDA&& Lambda) {
		using ContextSemanticsImpl = TContextSemanticsImpl<void>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<typename std::decay<decltype(Lambda)>::type>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const&, StorageType const& Storage, PARAMS... Params) {
			return StorageSemanticsImpl::Get(Storage)(std::forward<PARAMS>(Params)...);
		};

		StorageSemanticsImpl::Construct(NewDelegate.Storage, std::forward<TLAMBDA>(Lambda));
		return NewDelegate;
	}
	/** Create using a lambda called with a raw context object */
	template<typename C, typename TLAMBDA>
	static Delegate Create(C* Instance, TLAMBDA&& Lambda) {
		using ContextSemanticsImpl = TContextSemanticsImpl<typename std::conditional<std::is_const<C>::value, void const*, void*>::type>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<typename std::decay<decltype(Lambda)>::type>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const& Context, StorageType const& Storage, PARAMS... Params) {
			return StorageSemanticsImpl::Get(Storage)(ContextSemanticsImpl::Get(Context), std::forward<PARAMS>(Params)...);
		};

		ContextSemanticsImpl::Construct(NewDelegate.Context, Instance);
		StorageSemanticsImpl::Construct(NewDelegate.Storage, std::forward<TLAMBDA>(Lambda));
		return NewDelegate;
	}
		/** Create using a lambda called with a smart context object */
	template<typename C, typename TLAMBDA>
	static Delegate Create(std::weak_ptr<C>&& Instance, TLAMBDA&& Lambda) {
		using ContextSemanticsImpl = TContextSemanticsImpl<typename std::conditional<std::is_const<C>::value, std::weak_ptr<void const>, std::weak_ptr<void>>::type>;
		using StorageSemanticsImpl = TStorageSemanticsImpl<typename std::decay<decltype(Lambda)>::type>;

		Delegate NewDelegate;
		NewDelegate.Semantics = StaticCreateWrappedSemantics<ContextSemanticsImpl, StorageSemanticsImpl>();
		NewDelegate.StubFunction = [](ContextType const& Context, StorageType const& Storage, PARAMS... Params) {
			if( const std::shared_ptr<C> LockedContext = ContextSemanticsImpl::Get(Context).lock()) {
				return StorageSemanticsImpl::Get(Storage)(LockedContext.get(), std::forward<PARAMS>(Params)...);
			} else {
				return R{};
			}
		};

		ContextSemanticsImpl::Construct(NewDelegate.Context, std::forward<std::weak_ptr<C>>(Instance));
		StorageSemanticsImpl::Construct(NewDelegate.Storage, std::forward<TLAMBDA>(Lambda));
		return NewDelegate;
	}

	/** Executes the delegate with the provided parameters */
	R Execute(PARAMS... Params) const {
		return StubFunction(Context, Storage, std::forward<PARAMS>(Params)...);
	}
	/** Unbind the delegate so that it no longer executes any specific behavior, essentially returning it to a default-constructed state. */
	void Reset() {
		*this = Delegate{};
	}
};

/** A delegate which has no parameters and returns nothing */
using SimpleDelegate = Delegate<void>;
