#include <iostream>
#include "Logger.h"

void StandardLogger::Debug( char const* M )
{
	if( CurrentLevel > ELogLevel::Debug ) return;
	TerminalOutput( TERM_Cyan, M );
}

void StandardLogger::Message( char const* M )
{
	if( CurrentLevel > ELogLevel::Message ) return;
	TerminalOutput( "", M );
}

void StandardLogger::Warning( char const* M )
{
	if( CurrentLevel > ELogLevel::Warning ) return;
	TerminalOutput( TERM_Yellow "Warning: ", M );
}

void StandardLogger::Error( char const* M )
{
	if( CurrentLevel > ELogLevel::Error ) return;
	TerminalOutput( TERM_Red "Error: ", M );
}

void StandardLogger::TerminalOutput( char const* Prefix, char const* M )
{
	std::cout << Prefix << M << TERM_NoColor << std::endl;
}
