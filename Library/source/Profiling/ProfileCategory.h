#pragma once
#include "Engine/STL.h"

struct ProfileCategory {
private:
	std::string_view name;

public:
	ProfileCategory(std::string_view inName)
	: name(inName)
	{}

	inline std::string_view GetName() const { return name; };
};

#define DECLARE_PROFILE_CATEGORY(Name)\
extern ProfileCategory Profile ## Name

#define DEFINE_PROFILE_CATEGORY(Name)\
ProfileCategory Profile ## Name{ #Name };

DECLARE_PROFILE_CATEGORY(Default);
