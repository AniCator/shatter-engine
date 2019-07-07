// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <chrono>
#include <vector>
#include <map>
#include <atomic>

#include <Engine/Utility/RingBuffer.h>
#include <Engine/Utility/Structures/Name.h>

template<typename T>
bool ExclusiveComparison( const T& A, const T& B )
{
	return A < B && !( A > B );
}

static const size_t TimeWindow = 512;
struct FProfileTimeEntry
{
	FProfileTimeEntry()
	{
		Name = FName();
		Time = 0;
		Depth = 0;
		StartTime = 0;
	}

	FProfileTimeEntry( const FName& NameIn, const int64_t& TimeIn, const int64_t& StartTimeIn, const size_t& DepthIn )
	{
		Name = NameIn;
		Time = TimeIn;
		StartTime = StartTimeIn;
		Depth = DepthIn;
	}

	FProfileTimeEntry( const FName& NameIn, const int64_t& TimeIn, const size_t& DepthIn )
	{
		Name = NameIn;
		Time = TimeIn;
		StartTime = 0;
		Depth = DepthIn;
	}

	FProfileTimeEntry( const FName& NameIn, const int64_t& TimeIn )
	{
		Name = NameIn;
		Time = TimeIn;
		StartTime = 0;
		Depth = 0;
	}

	bool operator<( const FProfileTimeEntry& Entry ) const;

	FName Name;
	int64_t Time;
	int64_t StartTime;
	size_t Depth;
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
	std::map<FName, CRingBuffer<FProfileTimeEntry, TimeWindow>> TimeEntries;

	std::map<FName, int64_t> TimeCounters;
	std::map<FName, int64_t> TimeCountersFrame;
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
	CTimerScope( const FName& ScopeNameIn, bool TextOnly = true );
	~CTimerScope();

private:
	FName ScopeName;
	std::chrono::steady_clock::time_point StartTime;

	bool TextOnly;

	static std::atomic<size_t> Depth;
};

#ifdef ProfileBuild
#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define _PROFILENAME_(x) CONCAT(x, __LINE__)
#define _PROFILE_(Name, Bare) static FName _PROFILENAME_(ScopeName_)( Name ); CTimerScope _PROFILENAME_(Scope_)( Name, Bare )

#define ProfileScope() _PROFILE_( __FUNCTION__, false )
#define Profile( Name ) _PROFILE_( Name, false )
#define ProfileBareScope() _PROFILE_( __FUNCTION__, true )
#define ProfileBare( Name ) _PROFILE_( Name, true )
#else
#define ProfileScope() (void(0))
#define Profile( Name ) (void(0))
#define ProfileBareScope() (void(0))
#define ProfileBare( Name ) (void(0))
#endif
