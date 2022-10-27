// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ThreadPool.h"

#include <Engine/Utility/Thread.h>

Worker* Pool[Thread::Maximum];

void ThreadPool::Initialize()
{
	Shutdown();

	for( size_t Index = 0; Index < Thread::Maximum; Index++ )
	{
		Pool[Index] = new Worker();
	}

	Pool[Thread::Loading]->SetName( "Loading" );
	Pool[Thread::WorkerA]->SetName( "WorkerA" );
	Pool[Thread::WorkerB]->SetName( "WorkerB" );
}

void ThreadPool::Shutdown()
{
	for( size_t Index = 0; Index < Thread::Maximum; Index++ )
	{
		delete Pool[Index];
	}
}

std::future<void> ThreadPool::Add( const Thread::Type& Thread, const std::shared_ptr<Task>& ToExecute )
{
	return Pool[Thread]->Add( ToExecute );
}

std::future<void> ThreadPool::Add( const Thread::Type& Thread, const std::function<void()>& ToExecute )
{
	const auto Task = std::make_shared<LambdaTask>( [ToExecute]
		{
			ToExecute();
		} 
	);

	return Pool[Thread]->Add( Task );
}

// Determines which worker is used next for a generic task.
std::atomic<bool> Selector;

std::future<void> ThreadPool::Add( const std::shared_ptr<Task>& ToExecute )
{
	const auto Worker = Selector ? Thread::WorkerA : Thread::WorkerB;
	Selector = !Selector;

	return Pool[Worker]->Add( ToExecute );
}

std::future<void> ThreadPool::Add( const std::function<void()>& ToExecute )
{
	const auto Task = std::make_shared<LambdaTask>( [ToExecute]
		{
			ToExecute();
		}
	);

	const auto Worker = Selector ? Thread::WorkerA : Thread::WorkerB;
	Selector = !Selector;

	return Pool[Worker]->Add( Task );
}

void ThreadPool::Flush()
{
	Pool[Thread::WorkerA]->Flush();
	Pool[Thread::WorkerB]->Flush();
}

bool ThreadPool::IsBusy( const Thread::Type& Thread )
{
	return !Pool[Thread]->Completed();
}
