// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Logging.h"

#include <iostream>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#endif

inline void BreakDebugger()
{
#ifdef _WIN32
	if( IsDebuggerPresent() )
	{
		__debugbreak();
	}
#endif
};

namespace Log
{
	static const int nMaximumLogMessageLength = 2048;

	void Event( const char* Format, ... )
	{
		va_list Arguments;
		va_start( Arguments, Format );
		CLog::GetInstance().Event( Format, Arguments );
		va_end( Arguments );
	}

	void Event( LogSeverity Severity, const char* Format, ... )
	{
		va_list Arguments;
		va_start( Arguments, Format );
		CLog::GetInstance().Event( Severity, Format, Arguments );
		va_end( Arguments );
	}

	CLog::CLog( const char* LogName )
	{
		strcpy_s( Name, LogName );
	}

	CLog::CLog()
	{
		Name[0] = '\0';
	}

	CLog::~CLog()
	{

	}

	void CLog::Print( const char* Format, va_list Arguments )
	{
		char FullMessage[nMaximumLogMessageLength];
		vsprintf_s( FullMessage, Format, Arguments );

		char LogMessage[nMaximumLogMessageLength];
		if( Name[0] != '\0' )
		{
			sprintf_s( LogMessage, "%s: %s", Name, FullMessage );
		}
		else
		{
			strcpy_s( LogMessage, FullMessage );
		}

#ifndef ConsoleWindowDisabled
		printf( LogMessage );
#else
		if( IsDebuggerPresent() )
		{
			OutputDebugString( LogMessage );
		}
#endif
	}

	void CLog::Event( const char* Format, va_list Arguments )
	{
		Print( Format, Arguments );
	}

	void CLog::Event( LogSeverity Severity, const char* Format, va_list Arguments )
	{
		Print( Format, Arguments );

		if( Severity >= Error )
		{
			BreakDebugger();
		}

		if( Severity >= Fatal )
		{
			exit( EXIT_FAILURE );
		}
	}
}
