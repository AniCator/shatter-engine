// Copyright © 2017, Christiaan Bakker, All rights reserved.

#pragma once

#include <functional>
#include <future>
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

	std::promise<void> Promise;
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
	Worker();
	~Worker();

	bool Completed() const;
	bool IsRunning() const;

	std::future<void> Add( const std::shared_ptr<Task>& ToExecute );

	void SetName( const std::string& Name );
	void SetPriority( const ThreadPriority& Priority );

	// Finish any work that has been assigned so far.
	void Flush();

private:
	void Work();
	std::shared_ptr<Task> Fetch();

	void RunNextTask();
	bool HasTasks();

	std::thread Thread;
	std::deque<std::shared_ptr<Task>> Tasks;

	std::mutex Mutex;
	std::condition_variable Notify;

	std::atomic<bool> Running;
	std::atomic<bool> Alive;
};