// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <chrono>
#include <vector>
#include <map>
#include <atomic>

#include <Engine/Utility/RingBuffer.h>
#include <Engine/Utility/Structures/Name.h>
#include <Engine/Utility/Singleton.h>

static const size_t TimeWindow = 512;
struct ProfileTimeEntry
{
	ProfileTimeEntry() = default;
	
	ProfileTimeEntry( const NameSymbol& NameIn, const int64_t& TimeIn, const int64_t& StartTimeIn, const size_t& DepthIn )
	{
		Name = NameIn;
		Time = TimeIn;
		StartTime = StartTimeIn;
		Depth = DepthIn;
	}

	ProfileTimeEntry( const NameSymbol& NameIn, const int64_t& TimeIn, const size_t& DepthIn )
	{
		Name = NameIn;
		Time = TimeIn;
		StartTime = 0;
		Depth = DepthIn;
	}

	ProfileTimeEntry( const NameSymbol& NameIn, const int64_t& TimeIn )
	{
		Name = NameIn;
		Time = TimeIn;
		StartTime = 0;
		Depth = 0;
	}

	bool operator<( const ProfileTimeEntry& Entry ) const;

	NameSymbol Name = NameSymbol::Invalid;
	int64_t Time = 0;
	int64_t StartTime = 0;
	size_t Depth = 0;
};

class CProfiler : public Singleton<CProfiler>
{
public:
	~CProfiler();

	void AddTimeEntry( const ProfileTimeEntry& TimeEntry );

	void AddCounterEntry( const ProfileTimeEntry& TimeEntry, const bool& PerFrame = false, const bool& Assign = false );
	void AddCounterEntry( const char* NameIn, int TimeIn );

	void AddDebugMessage( const char* NameIn, const char* Body );

	void AddMemoryEntry( const NameSymbol& Name, const size_t& Bytes );
	void ClearMemoryEntry( const NameSymbol& Name );

	void Display();
	void Clear();
	void ClearFrame();

	bool IsEnabled() const;
	void SetEnabled( const bool EnabledIn );

	static size_t GetMemoryUsageInBytes();
	static size_t GetMemoryUsageInKiloBytes();
	static size_t GetMemoryUsageInMegaBytes();
	static size_t GetMemoryUsageInGigaBytes();
	static std::string GetMemoryUsageAsString();

private:
	std::map<NameSymbol, RingBuffer<ProfileTimeEntry, TimeWindow>> TimeEntries;

	std::map<NameSymbol, int64_t> TimeCounters;
	std::map<NameSymbol, int64_t> TimeCountersFrame;
	std::map<std::string, std::string> DebugMessages;

	std::map<NameSymbol, size_t> MemoryCounters;
	bool FirstMemoryEntry = true;
	size_t MemoryStartBytes = 0;

#ifdef ProfileBuild
	bool Enabled = true;
#else
	bool Enabled = false;
#endif

	void PlotPerformance();
};

class TimerScope
{
public:
	TimerScope() = delete;
	TimerScope( const NameSymbol& ScopeNameIn, bool TextOnly = true );
	TimerScope( const NameSymbol& ScopeNameIn, const uint64_t& Delta );
	~TimerScope();

	static void Submit( const NameSymbol& ScopeNameIn, const std::chrono::steady_clock::time_point& StartTime, const uint64_t& Delta );

private:
	NameSymbol ScopeName = NameSymbol::Invalid;
	std::chrono::steady_clock::time_point StartTime;

	bool TextOnly;

	static std::atomic<size_t> Depth;
};

class ProfileMemory
{
public:
	ProfileMemory() = delete;
	ProfileMemory( const NameSymbol& ScopeNameIn, const bool& ClearPrevious );
	~ProfileMemory();
private:
	NameSymbol ScopeName = NameSymbol::Invalid;
	size_t MemoryStartSize = 0;
	bool Clear = false;
};

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define _PROFILENAME_(x) CONCAT(x, __LINE__)
#define _PROFILE_(Name, Bare) static NameSymbol _PROFILENAME_(ScopeName_)( Name ); TimerScope _PROFILENAME_(Scope_)( Name, Bare )

#define _PROFILEMEMORY_( Name, Clear ) static NameSymbol _PROFILENAME_(ScopeName_)( Name ); ProfileMemory _PROFILENAME_(Scope_)( Name, Clear )

#define ProfileAlways( Name ) _PROFILE_( Name, false )

#ifdef OptickBuild
#include <ThirdParty/Optick/optick.h>
#define ProfileScope() OPTICK_EVENT()
#define Profile( Name ) OPTICK_EVENT( Name )
#define ProfileBareScope() OPTICK_EVENT()
#define ProfileBare( Name ) OPTICK_EVENT()
#define ProfileThread( Name ) OPTICK_THREAD( Name )
#define ProfileFrame( Name ) OPTICK_FRAME( Name )

#define ProfileMemory( Name ) _PROFILEMEMORY_( Name, false )
#define ProfileMemoryClear( Name ) _PROFILEMEMORY_( Name, true )

#define OptickEvent( ... ) OPTICK_EVENT( ##__VA_ARGS__ )
#define OptickCategory( Name, Category ) OPTICK_CATEGORY( Name, Category )
#define OptickCallback( Function ) OPTICK_SET_STATE_CHANGED_CALLBACK( Function )
#define OptickStart() OPTICK_START_CAPTURE()
#define OptickStop() OPTICK_STOP_CAPTURE()
#define OptickSave( Path ) OPTICK_SAVE_CAPTURE( Path )
#elif ProfileBuild
#define ProfileScope() _PROFILE_( __FUNCTION__, false )
#define Profile( Name ) _PROFILE_( Name, false )
#define ProfileBareScope() _PROFILE_( __FUNCTION__, true )
#define ProfileBare( Name ) _PROFILE_( Name, true )
#define ProfileThread( Name ) (void(0))
#define ProfileFrame( Name ) (void(0))

#define ProfileMemory( Name ) _PROFILEMEMORY_( Name, false )
#define ProfileMemoryClear( Name ) _PROFILEMEMORY_( Name, true )

#define OptickEvent( ... ) (void(0))
#define OptickCategory() (void(0))
#define OptickCallback() (void(0))
#define OptickStart() (void(0))
#define OptickStop() (void(0))
#define OptickSave( Path ) (void(0))
#else
#define ProfileScope() (void(0))
#define Profile( Name ) (void(0))
#define ProfileBareScope() (void(0))
#define ProfileBare( Name ) (void(0))
#define ProfileThread( Name ) (void(0))
#define ProfileFrame( Name ) (void(0))

#define ProfileMemory( Name ) (void(0))
#define ProfileMemoryClear( Name ) (void(0))

#define OptickEvent( ... ) (void(0))
#define OptickCategory() (void(0))
#define OptickCallback() (void(0))
#define OptickStart() (void(0))
#define OptickStop() (void(0))
#define OptickSave( Path ) (void(0))
#endif
