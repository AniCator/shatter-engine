// Copyright © 2017, Christiaan Bakker, All rights reserved.

#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct Task
{
	virtual ~Task() {};
	virtual void Execute() = 0;
};

struct LambdaTask : public Task
{
	LambdaTask( const std::function<void()>& ToExecute )
	{
		Function = ToExecute;
	}

	void Execute() override
	{
		Function();
	}

private:
	std::function<void()> Function;
};

struct Worker
{
	Worker()
	{
		Ready = false;
		Running = false;
		Alive = true;
		Thread = std::thread( &Worker::Work, this );
	}

	~Worker()
	{
		Alive = false;
		Task = nullptr;
		Notify.notify_one();
		Thread.join();
	}

	bool Completed() const
	{
		return !Ready;
	}

	bool IsRunning() const
	{
		return Running;
	}

	void Start( const std::shared_ptr<Task>& ToExecute )
	{
		if( Ready )
			return;

		std::unique_lock<std::mutex> Lock( Mutex );

		Task = ToExecute;
		Ready = true;

		Notify.notify_one();
	}

	void SetName( const std::string& Name );

private:
	void Work();

	std::thread Thread;
	std::shared_ptr<Task> Task = nullptr;

	std::mutex Mutex;
	std::condition_variable Notify;

	std::atomic<bool> Ready;
	std::atomic<bool> Running;
	std::atomic<bool> Alive;
};