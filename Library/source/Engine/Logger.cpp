#include <iostream>
#include "Logger.h"

void StandardLogger::Debug( std::string_view Message )
{
	TerminalOutput( TERM_Cyan, Message );
}

void StandardLogger::Message( std::string_view Message )
{
	TerminalOutput( "", Message );
}

void StandardLogger::Warning( std::string_view Message )
{
	TerminalOutput( TERM_Yellow "Warning: ", Message );
}

void StandardLogger::Error( std::string_view Message )
{
	TerminalOutput( TERM_Red "Error: ", Message );
}

void StandardLogger::TerminalOutput( std::string_view Prefix, std::string_view Message )
{
	std::cout << Prefix << Message << TERM_NoColor << std::endl;
}
