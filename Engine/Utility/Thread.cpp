// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Thread.h"

#include <Engine/Profiling/Logging.h>
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

Worker::Worker()
{
	Running = false;

	// Make sure alive is set to true before starting the thread, otherwise it will exit instantly.
	Alive = true;

	// Create the worker thread.
	Thread = std::thread( &Worker::Work, this );
}

Worker::~Worker()
{
	// Signal to the worker that it is going to be shut down.
	Alive = false;
	Notify.notify_one();

	// Check if this worker has created a thread.
	if( Thread.joinable() )
	{
		// Join the thread.		
		Thread.join();
	}
}

bool Worker::Completed() const
{
	return !Running;
}

bool Worker::IsRunning() const
{
	return Running;
}

std::future<void> Worker::Add( const std::shared_ptr<Task>& ToExecute )
{
	// A lock is required because we're accessing and manipulating the queue's memory.
	std::unique_lock<std::mutex> Lock( Mutex );

	// Add the tasks to the queue.
	Tasks.emplace_back( ToExecute );
	Running = true;

	// Refresh the promise, in case this task is being re-used.
	Tasks.back()->Promise = std::promise<void>();

	// Notify the worker thread that there is new work.
	Notify.notify_one();

	// Return the associated future.
	return Tasks.back()->Promise.get_future();
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
	ProfileThread( "Shatter Engine Worker" );

	Log::Event( "Thread started.\n" );

	while( Alive )
	{
		RunNextTask();
	}

	Log::Event( "Thread finished.\n" );
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

		// Notify future.
		Task->Promise.set_value();
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
