// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Logging.h"

#include <iostream>
#include <chrono>
#include <atomic>

#include <Engine/Application/Application.h>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#endif

void BreakDebugger()
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
	static constexpr int MaximumLogMessageLength = 8192 * 2;
	static constexpr int PrintThreshold = 10;
	static std::atomic<int> PrintCount = 0;

	void Event( const char* Format, ... )
	{
		va_list Arguments;
		va_start( Arguments, Format );
		CLog::Get().Event( Format, Arguments );
		va_end( Arguments );
	}

	void Event( LogSeverity Severity, const char* Format, ... )
	{
		va_list Arguments;
		va_start( Arguments, Format );
		CLog::Get().Event( Severity, Format, Arguments );
		va_end( Arguments );
	}

	const std::vector<FHistory>& History()
	{
		return CLog::Get().History();
	}

	CLog::CLog( const char* LogName )
	{
		if( LogName[0] != '\0' )
		{
#if defined( _WIN32 )
			ToStdOut = false;
#else
			ToStdOut = true;
#endif

			strcpy_s( Name, LogName );

			const auto Directory = CApplication::UTF16ToUTF8( CApplication::GetUserSettingsDirectory() );
			LogPath = Directory + Name + ".log";
			ShatterLogPath = Directory + "ShatterGlobalLog.log";

			if( Name[0] != '\0' )
			{
				if( LogPath.length() > 0 )
				{
					LogOutputStream.open( LogPath, std::fstream::out | std::fstream::app );
				}
				else
				{
					char LogFileName[256];
					sprintf_s( LogFileName, "%s.log", Name );
					LogOutputStream.open( LogFileName, std::fstream::out | std::fstream::app );
				}
			}
			else
			{
				if( ShatterLogPath.length() > 0 )
				{
					LogOutputStream.open( ShatterLogPath, std::fstream::out | std::fstream::app );
				}
				else
				{
					LogOutputStream.open( "ShatterGlobalLog.log", std::fstream::out | std::fstream::app );
				}
			}

			Timer.Start();
		}
		else
		{
			CLog();
		}
	}

	CLog::CLog()
	{
#if defined( _WIN32 )
		ToStdOut = false;
#else
		ToStdOut = true;
#endif

		Name[0] = '\0';

		const auto Directory = CApplication::UTF16ToUTF8( CApplication::GetUserSettingsDirectory() );
		// LogPath = Directory + Name + ".log";
		ShatterLogPath = Directory + "ShatterGlobalLog.log";
		LogOutputStream.open( ShatterLogPath );

		Timer.Start();
	}

	CLog::~CLog()
	{
		Timer.Stop();
		LogOutputStream.close();
	}

	const std::vector<Log::FHistory>& CLog::History() const
	{
		return LogHistory;
	}

	void CLog::Print( LogSeverity Severity, const char* Format, va_list Arguments )
	{
		char FullMessage[MaximumLogMessageLength];
		vsprintf_s( FullMessage, Format, Arguments );

		char LogMessage[MaximumLogMessageLength];
		if( Name[0] != '\0' )
		{
			sprintf_s( LogMessage, "%s: %s", Name, FullMessage );
		}
		else
		{
			strcpy_s( LogMessage, FullMessage );
		}

		if( ToStdOut )
		{
			printf( LogMessage );
		}
		else
		{
			if( IsDebuggerPresent() )
			{
				OutputDebugString( LogMessage );
			}
		}

		LogMutex.lock();
		CLog& GlobalInstance = CLog::Get();
		if( this != &GlobalInstance )
		{
			GlobalInstance.PrintDirect( LogMessage );
		}

		FHistory NewHistory;
		NewHistory.Severity = Severity;
		NewHistory.Message = std::string( LogMessage );
		LogHistory.push_back( NewHistory );

		PrintDirect( LogMessage );
		LogMutex.unlock();
	}

	void CLog::PrintDirect( const char* Message )
	{
		if( LogOutputStream.is_open() )
		{
#if !defined(_DEBUG)
			char TimeCode[64];
			sprintf_s( TimeCode, "%.3fs", static_cast<float>( Timer.GetElapsedTimeMilliseconds() ) * 0.001f );
			LogOutputStream << TimeCode << ": " << Message;
#else
			LogOutputStream << Message;
#endif

			PrintCount++;

			if( PrintCount > PrintThreshold )
			{
				PrintCount = 0;
				LogOutputStream.close();

				if( Name[0] != '\0' )
				{
					if( LogPath.length() > 0 )
					{
						LogOutputStream.open( LogPath, std::fstream::out | std::fstream::app );
					}
					else
					{
						char LogFileName[256];
						sprintf_s( LogFileName, "%s.log", Name );
						LogOutputStream.open( LogFileName, std::fstream::out | std::fstream::app );
					}
				}
				else
				{
					if( ShatterLogPath.length() > 0 )
					{
						LogOutputStream.open( ShatterLogPath, std::fstream::out | std::fstream::app );
					}
					else
					{
						LogOutputStream.open( "ShatterGlobalLog.log", std::fstream::out | std::fstream::app );
					}
				}
			}
		}
	}

	void CLog::Event( const char* Format, ... )
	{
		va_list Arguments;
		va_start( Arguments, Format );
		Event( Format, Arguments );
		va_end( Arguments );
	}

	void CLog::Event( LogSeverity Severity, const char* Format, ... )
	{
		va_list Arguments;
		va_start( Arguments, Format );
		Event( Severity, Format, Arguments );
		va_end( Arguments );
	}

	void CLog::Event( const char* Format, va_list Arguments )
	{
		Event( Standard, Format, Arguments );
	}

	void CLog::Event( LogSeverity Severity, const char* Format, va_list Arguments )
	{
		Print( Severity, Format, Arguments );

		if( Severity >= Error && Severity != Assert )
		{
#ifdef _WIN32
			if( IsDebuggerPresent() )
			{
				__debugbreak();
			}
#endif
		}

		if( Severity >= Fatal )
		{
#ifdef _WIN32
			char FullMessage[MaximumLogMessageLength];
			vsprintf_s( FullMessage, Format, Arguments );

			const bool ValidName = Name[0] != '\0';

			static std::string FatalMessage = "Shatter Engine Fatal Error";

			const char* Caption = FatalMessage.c_str();
			if( Severity == Assert )
			{
				static std::string AssertMessage = "Shatter Engine Warning";
				Caption = AssertMessage.c_str();
			}

			MessageBox( nullptr, FullMessage, ValidName ? Name : Caption, MB_ICONERROR | MB_OK );
#endif

			if( Severity != Assert )
			{
				exit( EXIT_FAILURE );
			}
		}
	}

	std::vector<Log::FHistory> CLog::LogHistory;
}
