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
	if( !Handle )
		return;
	
	SetThreadName(
		::GetThreadId( static_cast<HANDLE>( Handle ) ),
		Name.c_str()
	);
}

void SetThreadPriority( std::thread& Thread, const ThreadPriority& Priority )
{
	auto* Handle = Thread.native_handle();
	if( !Handle )
		return;

	SetThreadPriority( static_cast<HANDLE>( Handle ), Priority );
}
#else
void SetThreadName( std::thread& Thread, const std::string& Name )
{

}

void SetThreadPriority( std::thread& Thread, const ThreadPriority& Priority )
{

}
#endif

void LambdaTask::Execute()
{
	Function();
}

void Worker::SetName( const std::string& Name )
{
	SetThreadName( Thread, Name );
}

void Worker::SetPriority( const ThreadPriority& Priority )
{
	SetThreadPriority( Thread, Priority );
}

void Worker::Flush()
{
	while( HasTasks() )
	{
		RunNextTask();
	}
}

void Worker::Work()
{
	ProfileThread( "Shatter Engine Worker");
	while( Alive )
	{
		RunNextTask();
	}
}

std::shared_ptr<Task> Worker::Fetch()
{
	std::unique_lock<std::mutex> Lock( Mutex );

	// Wait for a notification, or continue running until we've finished all our tasks.
	Notify.wait( Lock, [&] {
		return !Tasks.empty() || !Alive;
		}
	);

	Running = true;

	if( Tasks.empty() )
		return nullptr;

	// Grab a task, and pop it.
	auto Task = std::move( Tasks.front() );
	Tasks.pop_front();

	return std::move( Task );
}

void Worker::RunNextTask()
{
	const std::shared_ptr<Task> Task = Fetch();
	if( Task )
	{
		OptickEvent( "Task" );
		Task->Execute();
	}

	// If there's no more work to perform, we can declare the we're no longer actively running.
	std::unique_lock<std::mutex> Lock( Mutex );
	if( Tasks.empty() )
	{
		Running = false;
	}
}

bool Worker::HasTasks()
{
	std::unique_lock<std::mutex> Lock( Mutex );
	return !Tasks.empty();
}
