#pragma once
#include <string_view>

struct Logger
{
	virtual ~Logger() {}

	//Information messages
	virtual void Debug( std::string_view Message ) {}
	virtual void Message( std::string_view Message ) {}

	//Issue messages
	virtual void Warning( std::string_view Message ) {}
	virtual void Error( std::string_view Message ) {}
};

//A logger which prints to std::out
struct StandardLogger : public Logger
{
	virtual void Debug( std::string_view Message ) override;
	virtual void Message( std::string_view Message ) override;

	virtual void Warning( std::string_view Message ) override;
	virtual void Error( std::string_view Message ) override;

protected:
	void TerminalOutput( std::string_view Prefix, std::string_view Message );
};
