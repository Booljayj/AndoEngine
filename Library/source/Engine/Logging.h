#pragma once
#include "Engine/Logging/LogCategory.h"
#include "Engine/Logging/LogConfig.h"
#include "Engine/Logging/LogDevice.h"
#include "Engine/Logging/Logger.h"
#include "Engine/Logging/LogMessage.h"
#include "Engine/Logging/LogUtility.h"

//============================================================
// Log commands

/** Log a message with a particular category and verbosity */
#define LOG(Category, Verbosity, Message, ...)\
if constexpr (LogConfig::IsCompiled(ELogVerbosity::Verbosity)) {\
	Logger::Get().Push(Log ## Category, ELogVerbosity::Verbosity, LogUtility::GetSourceLocation(), Message, __VA_ARGS__);\
}

//============================================================
// Log category creation

/** Declare a log category, which is accessible from this location */
#define DECLARE_LOG_CATEGORY(Name)\
extern LogCategory Log ## Name

/** Define a log category that was previously declared */
#define DEFINE_LOG_CATEGORY(Name, DefaultMaxVerbosity)\
LogCategory Log ## Name{ #Name, ELogVerbosity::DefaultMaxVerbosity }

/** Declare a log category, which is accessible from this location as a static struct member */
#define DECLARE_LOG_CATEGORY_MEMBER(Name)\
static LogCategory Log ## Name

/** Define a log category that was previously declared */
#define DEFINE_LOG_CATEGORY_MEMBER(Class, Name, DefaultMaxVerbosity)\
LogCategory Class::Log ## Name{ #Name, ELogVerbosity::DefaultMaxVerbosity }

/** Create a log category which is accessible only from within a source file */
#define LOG_CATEGORY(Name, DefaultMaxVerbosity)\
LogCategory Log ## Name{ #Name, ELogVerbosity::DefaultMaxVerbosity };
