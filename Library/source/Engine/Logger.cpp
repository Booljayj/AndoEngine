#include <iostream>
#include "Logger.h"

using namespace std;

void StandardLogger::VeryVerbose( const char* M )
{
	if( CurrentLevel > ELogLevel::VeryVerbose ) return;
	TerminalOutput( TERM_Blue, M );
}

void StandardLogger::Verbose( const char* M )
{
	if( CurrentLevel > ELogLevel::Verbose ) return;
	TerminalOutput( TERM_Cyan, M );
}

void StandardLogger::Message( const char* M )
{
	if( CurrentLevel > ELogLevel::Message ) return;
	TerminalOutput( "", M );
}

void StandardLogger::Warning( const char* M )
{
	if( CurrentLevel > ELogLevel::Warning ) return;
	TerminalOutput( TERM_Yellow "Warning: ", M );
}

void StandardLogger::Error( const char* M )
{
	if( CurrentLevel > ELogLevel::Error ) return;
	TerminalOutput( TERM_Red "Error: ", M );
}

void StandardLogger::Fatal( const char* M )
{
	TerminalOutput( TERM_Red "Fatal: ", M );
}

void StandardLogger::TerminalOutput( const char* Prefix, const char* M )
{
	cout << Prefix << M << TERM_NoColor << endl;
}
