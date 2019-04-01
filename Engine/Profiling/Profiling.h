// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <chrono>
#include <vector>
#include <map>

#include <Engine/Utility/RingBuffer.h>

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

class CProfiler
{
public:
	~CProfiler();

	void AddTimeEntry( FProfileTimeEntry& TimeEntry );
	void AddCounterEntry( FProfileTimeEntry& TimeEntry, const bool PerFrame = false );
	void AddCounterEntry( const char* NameIn, int TimeIn );
	void AddDebugMessage( const char* NameIn, const char* Body );
	void Display();
	void Clear();
	void ClearFrame();

	bool IsEnabled() const;
	void SetEnabled( const bool EnabledIn );

private:
	std::map<std::string, CRingBuffer<int64_t, TimeWindow>> TimeEntries;
	std::map<std::string, int64_t> TimeCounters;
	std::map<std::string, int64_t> TimeCountersFrame;
	std::map<std::string, std::string> DebugMessages;

	bool Enabled;

	void PlotPerformance();

public:
	static CProfiler& Get()
	{
		static CProfiler StaticInstance;
		return StaticInstance;
	}
private:
	CProfiler();

	CProfiler( CProfiler const& ) = delete;
	void operator=( CProfiler const& ) = delete;
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
#define ProfileScope() CTimerScope Scope_( __FUNCTION__, false )
#define Profile( Name ) CTimerScope Scope_( Name, false )
#define ProfileBareScope() CTimerScope Scope_( __FUNCTION__, true )
#define ProfileBare( Name ) CTimerScope Scope_( Name, true )
#else
#define ProfileScope() (void(0))
#define Profile( Name ) (void(0))
#define ProfileBareScope() (void(0))
#define ProfileBare( Name ) (void(0))
#endif
