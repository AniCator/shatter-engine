// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <chrono>

class CTimer
{
public:
	CTimer( bool UpdateOnGetElapsed = false );
	~CTimer();

	void Start();
	void Stop();

	bool Enabled() const;

	int64_t GetElapsedTimeMicroseconds();
	int64_t GetElapsedTimeMilliseconds();
	double GetElapsedTimeSeconds();

private:
	std::chrono::steady_clock::time_point StartTime;
	std::chrono::steady_clock::time_point StopTime;

	bool IsRunning;
	bool UpdatedOnGetElapsed;
};
