#include "Engine/Logging/Logger.h"
#include "Engine/STL.h"

/** Writes output to a file */
struct FileLoggerModule : public LoggerModule {
public:
	FileLoggerModule(std::string_view fileName);
	virtual void ProcessMessage(LogOutputData const& outputData) override;

protected:
	std::fstream stream;
};
