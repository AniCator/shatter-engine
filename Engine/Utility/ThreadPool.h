// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
#include <future>
#include <memory>

struct Task;

namespace Thread
{
	enum Type
	{
		Loading,
		WorkerA,
		WorkerB,
		Maximum
	};
};

namespace ThreadPool
{
	void Initialize();
	void Shutdown();

	// Adds a task to the given thread.
	std::future<void> Add( const Thread::Type& Thread, const std::shared_ptr<Task>& ToExecute );

	// Creates a task for the given thread using the provided function.
	std::future<void> Add( const Thread::Type& Thread, const std::function<void()>& ToExecute );

	// Adds a task to one of the worker threads.
	std::future<void> Add( const std::shared_ptr<Task>& ToExecute );

	// Creates a task for one of the worker threads using the provided function.
	std::future<void> Add( const std::function<void()>& ToExecute );

	// Finish all work that has been assigned to the worker threads.
	void Flush();

	bool IsBusy( const Thread::Type& Thread );
}