// Copyright © 2017, Christiaan Bakker, All rights reserved.

#pragma once

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

struct Task
{
	Task()
	{
		// Default constructor is left empty.
	}

	Task(std::function<void()> ToExecute)
	{
		Function = ToExecute;
	}

	void Execute()
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
		Thread = std::thread(&Worker::Work, this);
	}

	~Worker()
	{
		Alive = false;
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

	void Start( const Task& ToExecute )
	{
		std::unique_lock<std::mutex> Lock( Mutex );

		Task = ToExecute;
		Ready = true;

		Notify.notify_one();
	}

private:
	void Work()
	{
		while (Alive)
		{
			std::unique_lock<std::mutex> Lock(Mutex);
			Notify.wait(Lock);

			Running = true;

			Task.Execute();

			Running = false;
			Ready = false;
		}
	}

	std::thread Thread;
	Task Task;

	std::mutex Mutex;
	std::condition_variable Notify;

	std::atomic<bool> Ready;
	std::atomic<bool> Running;
	std::atomic<bool> Alive;
};