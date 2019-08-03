#include <mutex>
#include <vector>
#include "Engine/Delegates.h"

using EventHandleType = std::shared_ptr<char const>;

template<typename... PARAMS>
class Event {
public:
	using DelegateType = Delegate<void, PARAMS...>;

private:
	struct DelegateInfo {
		EventHandleType Handle;
		DelegateType Delegate;
	};

	//Specific byte value used by handles so that they may be identified in debugging utilities.
	static constexpr char HandleByteValue = 0b10101010;

	std::vector<DelegateInfo> DelegateInfos;
	std::mutex EventMutex;

public:
	size_t Count() const {
		std::lock_guard<std::mutex> Guard(EventMutex);
		return DelegateInfos.size();
	}

	EventHandleType Add(DelegateType const& Delegate) {
		if(Delegate.IsBound()) {
			std::lock_guard<std::mutex> Guard(EventMutex);
			DelegateInfos.emplace_back(std::make_shared<char>(HandleByteValue), Delegate);
			return DelegateInfos.back().Handle;
		}
		return EventHandleType{};
	}
	EventHandleType Add(DelegateType&& Delegate) {
		if(Delegate.IsBound()) {
			std::lock_guard<std::mutex> Guard(EventMutex);
			DelegateInfos.emplace_back(std::make_shared<char>(HandleByteValue), std::move(Delegate));
			return DelegateInfos.back().Handle;
		}
		return EventHandleType{};
	}

	bool Remove(EventHandleType const& Handle) {
		std::lock_guard<std::mutex> Guard(EventMutex);
		for(size_t Index = 0; Index < DelegateInfos.size(); ++Index) {
			if(DelegateInfos[Index].Handle == Handle) {
				std::iter_swap(DelegateInfos.begin() + Index, DelegateInfos.end() - 1);
				DelegateInfos.pop_back();
				return true;
			}
		}
		return false;
	}
	bool RemoveAll() {
		std::lock_guard<std::mutex> Guard(EventMutex);
		DelegateInfos.clear();
	}

	void operator()(PARAMS... Params) const {
		std::lock_guard<std::mutex> Guard(EventMutex);
		for(size_t Index = 0; Index < DelegateInfos.size(); ++Index) {
			DelegateInfos[Index].Delegate(Params...);
		}
	}
};
