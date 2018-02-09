// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <chrono>
#include <vector>
#include <map>

#include <Utility/RingBuffer.h>

static const size_t TimeWindow = 512;

struct FProfileTimeEntry
{
	FProfileTimeEntry( const char* NameIn, int64_t& TimeIn )
	{
		strcpy_s( Name, NameIn );
		Time = TimeIn;
	}

	char Name[256];
	int64_t Time;
};

class CProfileVisualisation
{
public:
	~CProfileVisualisation();

	void AddTimeEntry( FProfileTimeEntry& TimeEntry );
	void AddCounterEntry( FProfileTimeEntry& TimeEntry );
	void AddCounterEntry( const char* NameIn, int TimeIn );
	void Display();
	void Clear();

	bool IsEnabled() const;
	void SetEnabled( const bool EnabledIn );

private:
	std::map<std::string, CRingBuffer<int64_t, TimeWindow>> TimeEntries;
	std::map<std::string, int64_t> TimeCounters;

	bool Enabled;

public:
	static CProfileVisualisation& GetInstance()
	{
		static CProfileVisualisation StaticInstance;
		return StaticInstance;
	}
private:
	CProfileVisualisation();

	CProfileVisualisation( CProfileVisualisation const& ) = delete;
	void operator=( CProfileVisualisation const& ) = delete;
};

class CTimerScope
{
public:
	CTimerScope( const char* ScopeNameIn, bool TextOnly = true );
	~CTimerScope();

private:
	char ScopeName[128];
	std::chrono::steady_clock::time_point StartTime;

	bool TextOnly;
};

#ifdef ProfileBuild
#define Profile( Name ) CTimerScope Scope_( Name, false )
#define ProfileBare( Name ) CTimerScope Scope_( Name, true )
#else
#define Profile( Name ) (void(0))
#define ProfileBare( Name ) (void(0))
#endif

class CTimer
{
public:
	CTimer( bool UpdateOnGetElapsed = false );
	~CTimer();

	void Start();
	void Stop();

	int64_t GetElapsedTimeMicroseconds();
	int64_t GetElapsedTimeMilliseconds();
	double GetElapsedTimeSeconds();

private:
	std::chrono::steady_clock::time_point StartTime;
	std::chrono::steady_clock::time_point StopTime;

	bool IsRunning;
	bool UpdatedOnGetElapsed;
};
