// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Timer.h>

#include <stdarg.h>
#include <fstream>

#ifdef _WIN32
#define ConsoleWindowDisabled
#endif

inline void BreakDebugger();

namespace Log
{
	enum LogSeverity
	{
		Normal = 0,
		Warning,
		Error,
		Fatal
	};

	void Event( const char* Format, ... );
	void Event( LogSeverity Severity, const char* Format, ... );

	class CLog
	{
	public:
		CLog( const char* LogName );
		~CLog();

		static CLog& GetInstance()
		{
			static CLog StaticInstance;
			return StaticInstance;
		}

		void Event( const char* Format, va_list Arguments );
		void Event( LogSeverity Severity, const char* Format, va_list Arguments );

	private:
		CLog();

		void Print( const char* Format, va_list Arguments );

		char Name[128];
		std::ofstream LogOutputStream;

		CTimer Timer;
	};
}