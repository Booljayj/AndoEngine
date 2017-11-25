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
#include <queue>
#include "Engine/LinearAllocator.h"

// Sequence Containers

template< class T >
using l_vector = std::vector<T, TLinearAllocator<T>>;

template< class T >
using l_deque = std::deque<T, TLinearAllocator<T>>;

template< class T >
using l_forward_list = std::forward_list<T, TLinearAllocator<T>>;

template< class T >
using l_list = std::list<T, TLinearAllocator<T>>;

// Associative Containers

template< class T >
using l_set = std::set<T, std::less<T>, TLinearAllocator<T>>;
template< class T >
using l_multiset = std::multiset<T, std::less<T>, TLinearAllocator<T>>;

template< class Key, class Value >
using l_map = std::map< Key, Value, std::less<Key>, TLinearAllocator<std::pair<const Key, Value>>>;
template< class Key, class Value >
using l_multimap = std::multimap< Key, Value, std::less<Key>, TLinearAllocator<std::pair<const Key, Value>>>;

// Unordered Associative Containers

template< class T >
using l_unordered_set = std::unordered_set<T, std::hash<T>, std::equal_to<T>, TLinearAllocator<T>>;
template< class T >
using l_unordered_multiset = std::unordered_multiset<T, std::hash<T>, std::equal_to<T>, TLinearAllocator<T>>;

template< class Key, class Value >
using l_unordered_map = std::unordered_map< Key, Value, std::hash<Key>, std::equal_to<Key>, TLinearAllocator<std::pair<const Key, Value>>>;
template< class Key, class Value >
using l_unordered_multimap = std::unordered_multimap< Key, Value, std::hash<Key>, std::equal_to<Key>, TLinearAllocator<std::pair<const Key, Value>>>;

// Container Adapters

template< class T >
using l_stack = std::stack<T, l_deque<T>>;

template< class T >
using l_queue = std::queue<T, l_deque<T>>;

template< class T >
using l_priority_queue = std::priority_queue<T, l_deque<T>>;
