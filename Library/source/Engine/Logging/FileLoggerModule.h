#include <fstream>
#include <string_view>
#include "Engine/Logging/Logger.h"

struct FileLoggerModule : public LoggerModule {
public:
	FileLoggerModule( std::string_view FileName );

protected:
	std::fstream Stream;
	virtual void InternalProcessMessage( LogOutputData const& OutputData ) override;
};
