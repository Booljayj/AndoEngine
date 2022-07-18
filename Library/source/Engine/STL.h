#pragma once

//Basics
#include <cassert>
#include <type_traits>

//Primitive types
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
using namespace std::string_view_literals;

//Array types
#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <queue>
#include <stack>
#include <vector>

//Map types
#include <map>
#include <unordered_map>

//Set types
#include <set>
#include <unordered_set>

//Utility types
#include <tuple>
#include <optional>
#include <variant>

//Smart Pointers
#include <memory>

//Streams
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

//Threading
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>

//Localization
#include <locale>
#include <codecvt>

//Misc
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <numeric>
#include <typeinfo>
#include <utility>

//Third-party and upcoming
#include "ThirdParty/uuid.h"

//Custom extensions
namespace stdext {
	class shared_recursive_mutex : public std::shared_mutex {
	public:
		inline void lock(void) {
			std::thread::id this_id = std::this_thread::get_id();
			if (owner == this_id) {
				// recursive locking
				count++;
			} else {
				// normal locking
				shared_mutex::lock();
				owner = this_id;
				count = 1;
			}
		}

		inline void unlock(void) {
			if(count > 1) {
				// recursive unlocking
				count--;
			} else {
				// normal unlocking
				owner = std::thread::id();
				count = 0;
				shared_mutex::unlock();
			}
		}

	private:
		std::atomic<std::thread::id> owner;
		uint32_t count;
	};
}
