// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <chrono>

class Timer
{
public:
	Timer( bool UpdateOnGetElapsed = false );
	~Timer();

	void Start();
	void Start( const uint64_t& Offset );
	void Start( const std::chrono::milliseconds& Offset );
	void Stop();

	bool Enabled() const;

	int64_t GetElapsedTimeNanoseconds();
	int64_t GetElapsedTimeMicroseconds();
	int64_t GetElapsedTimeMilliseconds();
	double GetElapsedTimeSeconds();

	std::chrono::steady_clock::time_point GetStartTime();
	std::chrono::steady_clock::time_point GetStopTime();

private:
	std::chrono::steady_clock::time_point StartTime;
	std::chrono::steady_clock::time_point StopTime;

	bool IsRunning;
	bool UpdatedOnGetElapsed;
};
