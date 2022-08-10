// Copyright © 2017, Christiaan Bakker, All rights reserved.

#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <deque>

void SetThreadName( std::thread& Thread, const std::string& Name );

enum ThreadPriority
{
	Lowest = -2,
	BelowNormal,
	Normal,
	AboveNormal,
	Highest,
};

void SetThreadPriority( std::thread& Thread, const ThreadPriority& Priority );

struct Task
{
	virtual void Execute() = 0;
};

struct LambdaTask : Task
{
	LambdaTask( const std::function<void()>& ToExecute )
	{
		Function = ToExecute;
	}

	void Execute() override;

private:
	std::function<void()> Function;
};

struct Worker
{
	Worker()
	{
		Running = false;
		Alive = true;
		Thread = std::thread( &Worker::Work, this );
	}

	~Worker()
	{
		Alive = false;
		Notify.notify_one();
		Thread.join();
	}

	bool Completed() const
	{
		return !Running;
	}

	bool IsRunning() const
	{
		return Running;
	}

	bool Add( const std::shared_ptr<Task>& ToExecute )
	{
		// A lock is required because we're accessing and manipulating the queue's memory.
		std::unique_lock<std::mutex> Lock( Mutex );

		// Add the tasks to the queue.
		Tasks.emplace_back( ToExecute );
		Running = true;

		// Notify the worker thread that there is new work.
		Notify.notify_one();

		return true;
	}

	void SetName( const std::string& Name );
	void SetPriority( const ThreadPriority& Priority );

	// Finish any work that has been assigned so far.
	void Flush();

private:
	void Work();
	std::shared_ptr<Task> Fetch();

	void RunNextTask();

	std::thread Thread;
	std::deque<std::shared_ptr<Task>> Tasks;

	std::mutex Mutex;
	std::condition_variable Notify;

	std::atomic<bool> Running;
	std::atomic<bool> Alive;
};