#pragma once
#include "Engine/ArrayView.h"
#include "Engine/StandardTypes.h"
#include "Engine/Reflection/Components/ArgumentInfo.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	//@todo Experimental. The main reason to have this is for function invocation, and that seems to be a hacky,
	// non-typesafe mess at the moment. Needs to be thought out more to figure out if there's a better way.

	//@todo It would be FANTASTIC if we could say that arguments must be const, because we don't have to worry about converting to non-const void*
	// That does of course mean that functions may only have a single return value, but that might be good for a lot of other reasons. If you
	// need multiple returns, you return a struct.

	struct TypeInfo;

	enum class EFunctionFlags : uint8_t {
		Static,
		Const,
		Hidden,
	};
	using FFunctionFlags = TFlags<EFunctionFlags>;

	struct ArgumentView : TArrayView<ArgumentInfo const*> {
		ArgumentInfo const* Find(Hash32 id) const {
			//@todo If we have a way to ensure that the field array is sorted, we can use a binary search here for better speed.
			const auto iter = std::find_if(this->begin(), this->end(), [=](ArgumentInfo const* info) { return info->id == id; });
			if (iter != this->end()) return *iter;
			else return nullptr;
		}
	};

	struct FunctionInfo {
		std::string name;
		std::string description;

		TypeInfo const* instanceType = nullptr;
		TypeInfo const* returnType = nullptr;
		TypeInfo const* argumentsTupleType = nullptr;
		ArgumentView argumentsView;
		size_t argumentsSize;

		uint16_t NameHash = 0;
		FFunctionFlags Flags = FFunctionFlags::None();

		virtual TArrayView<char> CreateArguments() const = 0;
		virtual bool Invoke(void* ret, TArrayView<char> args) = 0;
	};

	template<typename T, typename Enable = void>
	struct TFunctionInfo {};

	//============================================================
	// Static functions

	template<typename ReturnType, typename... ArgTypes>
	struct TFunctionInfo<ReturnType(*)(ArgTypes...), typename std::enable_if<std::is_same_v<ReturnType, void>, void>::type> : public FunctionInfo {
		using ArgsTupleType = std::tuple<ArgTypes...>;
		using FunctionType = ReturnType(*)(ArgTypes...);

		FunctionType func;

		virtual bool Invoke(void* ret, TArrayView<char> args) override {
			if (args.size() != argumentsSize) return false;
			std::apply(func, *static_cast<ArgsTupleType const*>(args.begin()));
			return true;
		}
	};
	template<typename ReturnType, typename... ArgTypes>
	struct TFunctionInfo<ReturnType(*)(ArgTypes...), typename std::enable_if<!std::is_same_v<ReturnType, void>, void>::type> : public FunctionInfo {
		using ArgsTupleType = std::tuple<ArgTypes...>;
		using FunctionType = ReturnType(*)(ArgTypes...);

		FunctionType func;

		virtual bool Invoke(void* ret, TArrayView<char> args) override {
			if (!ret || args.size() != argumentsSize) return false;
			*static_cast<ReturnType*>(ret) = std::apply(Func, *static_cast<ArgsTupleType const*>(args.begin()));
			return true;
		}
	};

	//============================================================
	// Member functions

	template<typename InstanceType, typename ReturnType, typename... ArgTypes>
	struct TFunctionInfo<ReturnType (InstanceType::*)(ArgTypes...), typename std::enable_if<std::is_same_v<ReturnType, void>, void>::type> : public FunctionInfo {
		using ArgsTupleType = std::tuple<InstanceType*, ArgTypes...>;
		using FunctionType = ReturnType(InstanceType::*)(ArgTypes...);

		FunctionType func;

		virtual bool Invoke(void* ret, TArrayView<char> args) override {
			if (args.size() != argumentsSize) return false;
			std::apply(func, *static_cast<ArgsTupleType const*>(args.begin()));
			return true;
		}
	};

	template<typename InstanceType, typename ReturnType, typename... ArgTypes>
	struct TFunctionInfo<ReturnType (InstanceType::*)(ArgTypes...), typename std::enable_if<!std::is_same_v<ReturnType, void>, void>::type> : public FunctionInfo {
		using ArgsTupleType = std::tuple<InstanceType*, ArgTypes...>;
		using FunctionType = ReturnType(InstanceType::*)(ArgTypes...);

		FunctionType func;

		virtual bool Invoke(void* ret, TArrayView<char> args) override {
			if (args.size() != argumentsSize) return false;
			*static_cast<ReturnType*>(ret) = std::apply(func, *static_cast<ArgsTupleType const*>(args.begin()));
			return true;
		}
	};
}
