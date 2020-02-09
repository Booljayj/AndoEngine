#include <fstream>
#include <string_view>
#include "Engine/Logging/LoggerModule.h"

struct FileLoggerModule : public LoggerModule {
public:
	FileLoggerModule(std::string_view fileName);

protected:
	std::fstream stream;
	virtual void InternalProcessMessage(LogOutputData const& outputData) override;
};
