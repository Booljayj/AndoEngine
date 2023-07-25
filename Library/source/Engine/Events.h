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
		DelegateType del;
	};

	//Specific byte value used by handles so that they may be identified in debugging utilities.
	static constexpr char HandleByteValue = 0b00001111;

	std::vector<DelegateInfo> delegateInfos;
	mutable stdext::shared_recursive_mutex mutex;
	
public:
	/** Return the number of delegates bound to this event */
	size_t Count() const {
		std::shared_lock lock(mutex);
		return delegateInfos.size();
	}

	/** Add a delegate that will be executed when this event is broadcast */
	EventHandleType Add(DelegateType const& delegate) {
		if(delegate.IsBound()) {
			std::unique_lock lock(mutex);
			delegateInfos.emplace_back(std::make_shared<char>(HandleByteValue), delegate);
			return delegateInfos.back().handle;
		}
		return EventHandleType{};
	}
	/** Add a delegate that will be executed when this event is broadcast */
	EventHandleType Add(DelegateType&& delegate) {
		if(delegate.IsBound()) {
			std::unique_lock lock(mutex);
			delegateInfos.emplace_back(std::make_shared<char>(HandleByteValue), std::move(delegate));
			return delegateInfos.back().handle;
		}
		return EventHandleType{};
	}

	/** Add a free function */
	EventHandleType Add(void(*function)(ParamTypes...)) { return Add(DelegateType::Create(function)); }
	/** Add a free function called with a raw context object */
	template<typename ObjectType>
	EventHandleType Add(ObjectType* instance, void(*function)(ObjectType*, ParamTypes...)) { return Add(DelegateType::Create(instance, function)); }
	/** Add a free function called with a smart context object */
	template<typename ObjectType>
	EventHandleType Add(std::shared_ptr<ObjectType>&& instance, void(*function)(ObjectType*, ParamTypes...)) { return Add(DelegateType::Create(std::move(instance), function)); }

	/** Add a method called on a raw context object */
	template<typename ObjectType>
	EventHandleType Add(ObjectType* instance, void(ObjectType::* method)(ParamTypes...)) { return Add(DelegateType::Create(instance, method)); }
	/** Add a method called on a smart context object */
	template<typename ObjectType>
	EventHandleType Add(std::shared_ptr<ObjectType>&& instance, void(ObjectType::* method)(ParamTypes...)) { return Add(DelegateType::Create(std::move(instance), method)); }

	/** Add a lambda */
	template<typename LambdaType>
	EventHandleType Add(LambdaType&& lambda) { return Add(DelegateType::Create(std::move(lambda))); }
	/** Add a lambda called with a raw context object */
	template<typename ObjectType, typename LambdaType>
	EventHandleType Add(ObjectType* instance, LambdaType&& lambda) { return Add(DelegateType::Create(instance, std::move(lambda))); }
	/** Add a lambda called with a smart context object */
	template<typename ObjectType, typename LambdaType>
	EventHandleType Add(std::shared_ptr<ObjectType>&& instance, LambdaType&& lambda) { return Add(DelegateType::Create(std::move(instance), std::move(lambda))); }

	/** Remove a delegate from this event using the handle returned when adding it */
	bool Remove(EventHandleType const& handle) {
		std::unique_lock lock(mutex);
		for(size_t Index = 0; Index < delegateInfos.size(); ++Index) {
			if(delegateInfos[Index].handle == handle) {
				std::iter_swap(delegateInfos.begin() + Index, delegateInfos.end() - 1);
				delegateInfos.pop_back();
				return true;
			}
		}
		return false;
	}
	/** Remove all the delegates added to this event. After this is called they will not be executed when broadcasting the event. */
	bool RemoveAll() {
		std::unique_lock lock(mutex);
		delegateInfos.clear();
	}

	/** Execute all the delegates added to this event */
	void operator()(ParamTypes... params) const {
		Broadcast(std::forward<ParamTypes>(params)...);
	}

	/** Execute all the delegates added to this event */
	void Broadcast(ParamTypes... params) const {
		//Copy the delegates before executing them. This allows the event to be modified while it is being broadcast.
		std::vector<DelegateType> delegatesCopy;
		{
			std::unique_lock lock(mutex);

			delegatesCopy.reserve(delegateInfos.size());
			for (auto const& info : delegateInfos) {
				delegatesCopy.emplace_back(info.del);
			}
		}

		//Invoke the local copies of each delegate
		for (size_t Index = 0; Index < delegatesCopy.size(); ++Index) {
			delegatesCopy[Index](std::forward<ParamTypes>(params)...);
		}
	}
};

/** an event which has no parameters */
using SimpleEvent = TEvent<>;
