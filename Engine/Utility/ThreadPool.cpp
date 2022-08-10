// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ThreadPool.h"

#include <Engine/Utility/Thread.h>

Worker Pool[Thread::Maximum];

void ThreadPool::Initialize()
{
	Pool[Thread::Loading].SetName( "Loading" );
	Pool[Thread::WorkerA].SetName( "WorkerA" );
	Pool[Thread::WorkerB].SetName( "WorkerB" );
}

void ThreadPool::Shutdown()
{

}

void ThreadPool::Add( const Thread::Type& Thread, const std::shared_ptr<Task>& ToExecute )
{
	Pool[Thread].Add( ToExecute );
}

void ThreadPool::Add( const Thread::Type& Thread, const std::function<void()>& ToExecute )
{
	const auto Task = std::make_shared<LambdaTask>( [ToExecute]
		{
			ToExecute();
		} 
	);

	Pool[Thread].Add( Task );
}

// Determines which worker is used next for a generic task.
std::atomic<bool> Selector;

void ThreadPool::Add( const std::shared_ptr<Task>& ToExecute )
{
	const auto Worker = Selector ? Thread::WorkerA : Thread::WorkerB;
	Selector = !Selector;
	Pool[Worker].Add( ToExecute );
}

void ThreadPool::Add( const std::function<void()>& ToExecute )
{
	const auto Task = std::make_shared<LambdaTask>( [ToExecute]
		{
			ToExecute();
		}
	);

	const auto Worker = Selector ? Thread::WorkerA : Thread::WorkerB;
	Selector = !Selector;
	Pool[Worker].Add( Task );
}

void ThreadPool::Flush()
{
	Pool[Thread::WorkerA].Flush();
	Pool[Thread::WorkerB].Flush();
}

bool ThreadPool::IsBusy( const Thread::Type& Thread )
{
	return !Pool[Thread].Completed();
}
