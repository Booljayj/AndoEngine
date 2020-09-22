#include "Engine/Logging/LoggerModule.h"
#include "Engine/STL.h"

struct FileLoggerModule : public LoggerModule {
public:
	FileLoggerModule(std::string_view fileName);

protected:
	std::fstream stream;
	virtual void InternalProcessMessage(LogOutputData const& outputData) override;
};
