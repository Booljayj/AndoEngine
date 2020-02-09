#include <mutex>
#include <vector>
#include "Engine/Delegates.h"

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
	static constexpr char HandleByteValue = 0b10101010;

	std::vector<DelegateInfo> delegateInfos;
	std::mutex eventMutex;

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
