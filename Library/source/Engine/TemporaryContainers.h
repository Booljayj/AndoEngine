#pragma once
#include "Engine/List.h"
#include "Engine/Map.h"
#include "Engine/Set.h"
#include "Engine/Temporary.h"
#include "Engine/Vector.h"
#include "Engine/Queue.h"

// Sequence Containers
template<class T>
using t_vector = std::vector<T, TTemporaryAllocator<T>>;
template<class T>
using t_deque = std::deque<T, TTemporaryAllocator<T>>;
template<class T>
using t_forward_list = std::forward_list<T, TTemporaryAllocator<T>>;
template<class T>
using t_list = std::list<T, TTemporaryAllocator<T>>;

// Associative Containers
template<class T>
using t_set = std::set<T, std::less<T>, TTemporaryAllocator<T>>;
template<class T>
using t_multiset = std::multiset<T, std::less<T>, TTemporaryAllocator<T>>;
template<class K, class V>
using t_map = std::map<K, V, std::less<K>, TTemporaryAllocator<std::pair<const K, V>>>;
template<class K, class V>
using t_multimap = std::multimap<K, V, std::less<K>, TTemporaryAllocator<std::pair<const K, V>>>;

// Unordered Associative Containers
template<class T>
using t_unordered_set = std::unordered_set<T, std::hash<T>, std::equal_to<T>, TTemporaryAllocator<T>>;
template<class T>
using t_unordered_multiset = std::unordered_multiset<T, std::hash<T>, std::equal_to<T>, TTemporaryAllocator<T>>;
template<class K, class V>
using t_unordered_map = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, TTemporaryAllocator<std::pair<const K, V>>>;
template<class K, class V>
using t_unordered_multimap = std::unordered_multimap<K, V, std::hash<K>, std::equal_to<K>, TTemporaryAllocator<std::pair<const K, V>>>;

// Container Adapters
template<class T>
using t_stack = std::stack<T, t_deque<T>>;
template<class T>
using t_queue = std::queue<T, t_deque<T>>;
template<class T>
using t_priority_queue = std::priority_queue<T, t_deque<T>>;
