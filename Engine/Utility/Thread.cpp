// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Thread.h"

#include <Engine/Profiling/Profiling.h>

#if defined(_WIN32)
#include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)
void SetThreadName( DWORD dwThreadID, const char* threadName ) {
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try {
		RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), ( ULONG_PTR*) & info );
	}
	__except( EXCEPTION_EXECUTE_HANDLER ) {
	}
#pragma warning(pop)
}

void SetThreadName( std::thread& Thread, const std::string& Name )
{
	auto* Handle = Thread.native_handle();
	if( Handle )
	{
		SetThreadName(
			::GetThreadId( static_cast<HANDLE>( Handle ) ),
			Name.c_str()
		);
	}
}
#else
void SetThreadName( std::thread& Thread, const std::string& Name )
{

}
#endif

void Worker::SetName( const std::string& Name )
{
	SetThreadName( Thread, Name );
}

void Worker::Work()
{
	ProfileThread( "Shatter Engine Worker");
	while( Alive )
	{
		std::unique_lock<std::mutex> Lock( Mutex );
		Notify.wait( Lock );

		Running = true;

		if( Task )
		{
			Task->Execute();
		}

		Running = false;
		Ready = false;
	}
}
