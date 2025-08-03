#pragma once
#include <absl/functional/function_ref.h>

template<typename T>
using FunctionRef = absl::FunctionRef<T>;
