// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Timer.h>

#include <stdarg.h>
#include <fstream>
#include <vector>
#include <shared_mutex>

#if defined(_WIN32)
#define ConsoleWindowDisabled
#endif

void BreakDebugger();

namespace Log
{
	enum LogSeverity
	{
		Standard = 0, // Writes a standard message.
		Warning, // Writes a warning message.
		Error, // Debug breaks if attached and logs the error.
		Fatal, // Debug breaks if attached, displays a message box and exits the programs.
		Assert, // Logs the assertion message and displays a message box.

		LogMax
	};

	struct FHistory
	{
		FHistory()
		{
			Severity = Standard;
			Message = "";
		}

		LogSeverity Severity;
		std::string Message;
	};

	void Event( const char* Format, ... );
	void Event( LogSeverity Severity, const char* Format, ... );
	const std::vector<FHistory>& History();

	class CLog
	{
	public:
		CLog( const char* LogName );
		~CLog();

		static CLog& Get()
		{
			static CLog StaticInstance;
			return StaticInstance;
		}

		void Event( const char* Format, ... );
		void Event( LogSeverity Severity, const char* Format, ... );

		void Event( const char* Format, va_list Arguments );
		void Event( LogSeverity Severity, const char* Format, va_list Arguments );

		CLog( const CLog& ) = default;
		CLog& operator=( const CLog& ) = default;

		const std::vector<FHistory>& History() const;

		bool ToStdOut;

	private:
		CLog();

		void Print( LogSeverity Severity, const char* Format, va_list Arguments );
		void PrintDirect( const char* Message );

		char Name[128];
		std::ofstream LogOutputStream;

		std::string LogPath;
		std::string ShatterLogPath;

		Timer Timer;
		static std::vector<FHistory> LogHistory;

		mutable std::shared_mutex LogMutex;
	};
}
