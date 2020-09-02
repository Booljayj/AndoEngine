#pragma once
#include <vector>
#include <queue>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include "Engine/LinearAllocator.h"

// Sequence Containers
template<class T>
using l_vector = std::vector<T, TLinearAllocator<T>>;
template<class T>
using l_deque = std::deque<T, TLinearAllocator<T>>;
template<class T>
using l_forward_list = std::forward_list<T, TLinearAllocator<T>>;
template<class T>
using l_list = std::list<T, TLinearAllocator<T>>;

// Associative Containers
template<class T>
using l_set = std::set<T, std::less<T>, TLinearAllocator<T>>;
template<class T>
using l_multiset = std::multiset<T, std::less<T>, TLinearAllocator<T>>;
template<class KeyType, class ValueType>
using l_map = std::map<KeyType, ValueType, std::less<KeyType>, TLinearAllocator<std::pair<const KeyType, ValueType>>>;
template<class KeyType, class ValueType>
using l_multimap = std::multimap<KeyType, ValueType, std::less<KeyType>, TLinearAllocator<std::pair<const KeyType, ValueType>>>;

// Unordered Associative Containers
template<class T>
using l_unordered_set = std::unordered_set<T, std::hash<T>, std::equal_to<T>, TLinearAllocator<T>>;
template<class T>
using l_unordered_multiset = std::unordered_multiset<T, std::hash<T>, std::equal_to<T>, TLinearAllocator<T>>;
template<class KeyType, class ValueType>
using l_unordered_map = std::unordered_map<KeyType, ValueType, std::hash<KeyType>, std::equal_to<KeyType>, TLinearAllocator<std::pair<const KeyType, ValueType>>>;
template<class KeyType, class ValueType>
using l_unordered_multimap = std::unordered_multimap<KeyType, ValueType, std::hash<KeyType>, std::equal_to<KeyType>, TLinearAllocator<std::pair<const KeyType, ValueType>>>;

// Container Adapters
template<class T>
using l_stack = std::stack<T, l_deque<T>>;
template<class T>
using l_queue = std::queue<T, l_deque<T>>;
template<class T>
using l_priority_queue = std::priority_queue<T, l_deque<T>>;
