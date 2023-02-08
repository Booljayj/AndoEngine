#pragma once
#include "Engine/Delegates.h"
#include "Engine/StandardTypes.h"

using EventHandleType = std::shared_ptr<char const>;

template<typename... ParamTypes>
class TEvent {
public:
	using DelegateType = TDelegate<void, ParamTypes...>;

private:
	struct DelegateInfo {
		EventHandleType handle;
		DelegateType delegate;
	};

	//Specific byte value used by handles so that they may be identified in debugging utilities.
	static constexpr char HandleByteValue = 0b00001111;

	std::vector<DelegateInfo> delegateInfos;
	mutable std::mutex eventMutex; //@todo I don't like mutable here, but it seems to be an acceptable design pattern. Investigate ways we can get around this while still keeping the code clean.

public:
	size_t Count() const {
		std::lock_guard<std::mutex> guard(eventMutex);
		return delegateInfos.size();
	}

	EventHandleType Add(DelegateType const& delegate) {
		if(delegate.IsBound()) {
			std::lock_guard<std::mutex> guard(eventMutex);
			delegateInfos.emplace_back(std::make_shared<char>(HandleByteValue), delegate);
			return delegateInfos.back().handle;
		}
		return EventHandleType{};
	}
	EventHandleType Add(DelegateType&& delegate) {
		if(delegate.IsBound()) {
			std::lock_guard<std::mutex> guard(eventMutex);
			delegateInfos.emplace_back(std::make_shared<char>(HandleByteValue), std::move(delegate));
			return delegateInfos.back().handle;
		}
		return EventHandleType{};
	}

	/** Create using a free function */
	EventHandleType Add(void(*function)(ParamTypes...)) { return Add(DelegateType::Create(function)); }
	/** Create using a free function called with a raw context object */
	template<typename ObjectType>
	EventHandleType Add(ObjectType* instance, void(*function)(ObjectType*, ParamTypes...)) { return Add(DelegateType::Create(instance, function)); }
	/** Create using a free function called with a smart context object */
	template<typename ObjectType>
	EventHandleType Add(std::weak_ptr<ObjectType>&& instance, void(*function)(ObjectType*, ParamTypes...)) { return Add(DelegateType::Create(std::move(instance), function)); }

	/** Create using a method called on a raw context object */
	template<typename ObjectType>
	EventHandleType Add(ObjectType* instance, void(ObjectType::* method)(ParamTypes...)) { return Add(DelegateType::Create(instance, method)); }
	/** Create using a method called on a smart context object */
	template<typename ObjectType>
	EventHandleType Add(std::weak_ptr<ObjectType>&& instance, void(ObjectType::* method)(ParamTypes...)) { return Add(DelegateType::Create(std::move(instance), method)); }

	/** Create using a lambda */
	template<typename LambdaType>
	EventHandleType Add(LambdaType&& lambda) { return Add(DelegateType::Create(std::move(lambda))); }
	/** Create using a lambda called with a raw context object */
	template<typename ObjectType, typename LambdaType>
	EventHandleType Add(ObjectType* instance, LambdaType&& lambda) { return Add(DelegateType::Create(instance, std::move(lambda))); }
	/** Create using a lambda called with a smart context object */
	template<typename ObjectType, typename LambdaType>
	EventHandleType Add(std::weak_ptr<ObjectType>&& instance, LambdaType&& lambda) { return Add(DelegateType::Create(std::move(instance), std::move(lambda))); }

	bool Remove(EventHandleType const& handle) {
		std::lock_guard<std::mutex> guard(eventMutex);
		for(size_t Index = 0; Index < delegateInfos.size(); ++Index) {
			if(delegateInfos[Index].handle == handle) {
				std::iter_swap(delegateInfos.begin() + Index, delegateInfos.end() - 1);
				delegateInfos.pop_back();
				return true;
			}
		}
		return false;
	}
	bool RemoveAll() {
		std::lock_guard<std::mutex> guard(eventMutex);
		delegateInfos.clear();
	}

	void operator()(ParamTypes... params) const {
		std::lock_guard<std::mutex> guard(eventMutex);
		for(size_t Index = 0; Index < delegateInfos.size(); ++Index) {
			delegateInfos[Index].delegate(params...);
		}
	}
};

/** an event which has no parameters */
using SimpleEvent = TEvent<>;
