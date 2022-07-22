// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Profiling.h"
#include "Logging.h"

#include <algorithm>

#include <ThirdParty/imgui-1.70/imgui.h>

#include <Engine/Input/Input.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Math.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "Windows.h"
#include "Psapi.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#endif

const char* MouseLabels[] = 
{
	"Unknown",

	"MouseX",
	"MouseY",
	
	"MouseScrollUp",
	"MouseScrollDown",
	
	"LeftMouseButton",
	"RightMouseButton",
	"MiddleMouseButton",
	
	"MouseButton4",
	"MouseButton5",
	"MouseButton6",
	"MouseButton7",
	"MouseButton8",
	
	"Maximum"
};

const char* GamepadLabels[] =
{
	"Unknown",

	"GamepadFaceButtonUp",
	"GamepadFaceButtonDown",
	"GamepadFaceButtonLeft",
	"GamepadFaceButtonRight",

	"GamepadDirectionalButtonUp",
	"GamepadDirectionalButtonDown",
	"GamepadDirectionalButtonLeft",
	"GamepadDirectionalButtonRight",

	"GamepadLeftStickX",
	"GamepadLeftStickY",
	"GamepadLeftStickTrigger",

	"GamepadLeftTrigger",
	"GamepadLeftShoulder",
	"GamepadLeftSpecial",

	"GamepadRightStickX",
	"GamepadRightStickY",
	"GamepadRightStickTrigger",

	"GamepadRightTrigger",
	"GamepadRightShoulder",
	"GamepadRightSpecial",

	"Maximum"
};

// NOTE: Crappy memory tracker.
static size_t MemoryUsageBytes = 0;
static size_t LargestAllocation = 0;

// Only perform crappy memory tracking in non-release builds.
#ifndef ReleaseBuild
void* operator new( size_t Size )
{
	MemoryUsageBytes += Size;

	if( Size > LargestAllocation )
	{
		LargestAllocation = Size;
	}

	if( void* MemoryPointer = std::malloc( Size ) )
		return MemoryPointer;

	throw std::bad_alloc{};
}

void* operator new[]( size_t Size )
{
	// NOTE: We're not counting these allocations because we don't know for what size they're freed.
	if( void* MemoryPointer = std::malloc( Size ) )
		return MemoryPointer;

	throw std::bad_alloc{};
}

void operator delete( void* Data, size_t Size ) noexcept
{
	MemoryUsageBytes -= Size;

	std::free( Data );
}

void operator delete[]( void* Data, size_t Size ) noexcept
{
	// NOTE: We're not counting these allocations because we don't know for what size they're freed.
	std::free( Data );
}
#endif

#ifdef _WIN32
PROCESS_MEMORY_COUNTERS GetMemoryCounters()
{
	PROCESS_MEMORY_COUNTERS Memory;
	BOOL Result = GetProcessMemoryInfo(
		GetCurrentProcess(),
		&Memory,
		sizeof( Memory )
	);

	return Memory;
}

size_t GetMemoryUsageWorkingSet()
{
	const auto Memory = GetMemoryCounters();
	return Memory.WorkingSetSize;
}
#endif

size_t BytesToKiloBytes( const size_t& Bytes )
{
	return Bytes * 0.001;
}

size_t BytesToMegaBytes( const size_t& Bytes )
{
	return Bytes * 0.000001;
}

size_t BytesToGigaBytes( const size_t& Bytes )
{
	return Bytes * 0.000000001;
}

std::string BytesToString( const size_t& Bytes )
{
	std::string MemoryIdentifier = "GB";
	size_t Memory = BytesToGigaBytes( Bytes );
	if( Memory == 0 )
	{
		Memory = BytesToMegaBytes( Bytes );
		MemoryIdentifier = "MB";

		if( Memory == 0 )
		{
			Memory = BytesToKiloBytes( Bytes );
			MemoryIdentifier = "KB";

			if( Memory == 0 )
			{
				Memory = Bytes;
				MemoryIdentifier = "bytes";
			}
		}
	}

	return std::to_string( Memory ) + " " + MemoryIdentifier;
}

size_t MemoryCounterGap = 0;
size_t CounterGap = 0;
size_t CounterGapFrame = 0;

CProfiler::~CProfiler()
{
	Clear();
}

void CProfiler::AddTimeEntry( const ProfileTimeEntry& TimeEntry )
{
	auto Iterator = TimeEntries.find( TimeEntry.Name );
	if( Iterator == TimeEntries.end() )
	{
		RingBuffer<ProfileTimeEntry, TimeWindow> Buffer;
		Buffer.Insert( TimeEntry );
		TimeEntries.insert_or_assign( Iterator, TimeEntry.Name, Buffer );
	}
	else
	{
		Iterator->second.Insert( TimeEntry );
	}
}

void CProfiler::AddCounterEntry( const char* NameIn, int TimeIn )
{
	// if( !Enabled )
	// 	return;

	auto Iterator = TimeCounters.find( NameIn );
	if( Iterator == TimeCounters.end() )
	{
		TimeCounters.insert_or_assign( Iterator, NameIn, TimeIn );
	}
	else
	{
		Iterator->second += TimeIn;
	}
}

void CProfiler::AddCounterEntry( const ProfileTimeEntry& TimeEntry, const bool& PerFrame, const bool& Assign )
{
	if( !Enabled && PerFrame )
		return;

	if( PerFrame )
	{
		auto Iterator = TimeCountersFrame.find( TimeEntry.Name );
		if( Iterator == TimeCountersFrame.end() )
		{
			TimeCountersFrame.insert_or_assign( Iterator, TimeEntry.Name, TimeEntry.Time );
		}
		else
		{
			if( Assign )
			{
				Iterator->second = TimeEntry.Time;
			}
			else
			{
				Iterator->second += TimeEntry.Time;
			}
		}
	}
	else
	{
		auto Iterator = TimeCounters.find( TimeEntry.Name );
		if( Iterator == TimeCounters.end() )
		{
			TimeCounters.insert_or_assign( Iterator, TimeEntry.Name, TimeEntry.Time );
		}
		else
		{
			if( Assign )
			{
				Iterator->second = TimeEntry.Time;
			}
			else
			{
				Iterator->second += TimeEntry.Time;
			}
		}
	}
}

void CProfiler::AddDebugMessage( const char* NameIn, const char* Body )
{
	if( !Enabled )
		return;

	auto Iterator = DebugMessages.find( NameIn );
	if( Iterator == DebugMessages.end() )
	{
		DebugMessages.insert_or_assign( NameIn, Body );
	}
	else
	{
		Iterator->second = Body;
	}
}

void CProfiler::AddMemoryEntry( const NameSymbol& Name, const size_t& Bytes )
{
	// if( !Enabled )
	// 	return;

	if( FirstMemoryEntry )
	{
		FirstMemoryEntry = false;

		// The first memory entry should be ignored and just set the starting value.
		MemoryStartBytes = CProfiler::GetMemoryUsageInBytes();
	}

	const auto Iterator = MemoryCounters.find( Name );
	if( Iterator == MemoryCounters.end() )
	{
		MemoryCounters.insert_or_assign( Iterator, Name, Bytes );
	}
	else
	{
		Iterator->second += Bytes;
	}
}

void CProfiler::ClearMemoryEntry(const NameSymbol& Name)
{
	// if( !Enabled )
	// 	return;

	const auto Iterator = MemoryCounters.find( Name );
	if( Iterator == MemoryCounters.end() )
		return;

	MemoryCounters.erase( Iterator );
}

void CProfiler::PlotPerformance()
{
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	if( DrawList && Enabled )
	{
		const float SliceHeight = 15.0f;

		ImVec2 Position = ImGui::GetCursorScreenPos();
		ImVec2 Size = ImVec2( 500.0f, SliceHeight * TimeEntries.size() );
		if( Size.x < 50.0f ) Size.x = 50.0f;
		if( Size.y < 50.0f ) Size.y = 50.0f;
		DrawList->AddRectFilled( Position, ImVec2( Position.x + Size.x, Position.y + Size.y ), ImColor( 50, 50, 50 ), 2.0f);

		ImGui::InvisibleButton( "PlotPerformance", Size );

		DrawList->PushClipRect( Position, ImVec2( Position.x + Size.x, Position.y + Size.y ) );

		const auto CurrentTime = std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
		static std::vector<RingBuffer<ProfileTimeEntry, TimeWindow>*> PlottableEntries;
		PlottableEntries.clear();
		
		for( auto& Pair : TimeEntries )
		{
			auto& Buffer = Pair.second;
			const auto& TimeEntry = Buffer.Get( Buffer.Offset( -1 ) );
			const auto Difference = CurrentTime - TimeEntry.StartTime;
			if( Difference > 1000000000 )
				continue;

			PlottableEntries.emplace_back( &Buffer );
		}

		auto SortEntries = [&] ( RingBuffer<ProfileTimeEntry, TimeWindow>* A, RingBuffer<ProfileTimeEntry, TimeWindow>* B ) {
			const auto& TimeEntryA = A->Get( A->Offset( -1 ) );
			const auto& TimeEntryB = B->Get( B->Offset( -1 ) );
			return ExclusiveComparison( TimeEntryA.StartTime, TimeEntryB.StartTime ) && ExclusiveComparison( TimeEntryA.Depth, TimeEntryB.Depth );
		};

		std::sort( PlottableEntries.begin(), PlottableEntries.end(), SortEntries );

		int EntryIndex = 0;
		const int SliceAlpha = 255 / TimeEntries.size();
		static const float BarWindow = 1.0f / 24.0f * 1000.0f;

		char TimeEntryName[1024];
		for( auto& Buffer : PlottableEntries )
		{
			const auto& TimeEntry = Buffer->Get( Buffer->Offset( -1 ) );
			static const size_t AverageWindow = 128;
			float Average = 0.0f;
			int64_t Peak = 0;
			for( size_t j = 0; j < AverageWindow; j++ )
			{
				int64_t Time = Buffer->Get( Buffer->Offset( -static_cast<int>( j ) ) ).Time;
				Average += Time;

				if( Time > Peak )
				{
					Peak = Time;
				}
			}

			Average /= AverageWindow;

			bool Microsecond = false;
			bool Nanosecond = false;
			int64_t Time;
				Time = TimeEntry.Time / 1000000;
			if( Time == 0 )
			{
				Time = TimeEntry.Time / 100000;
				Microsecond = true;
			}

			if( Time == 0 )
			{
				Time = TimeEntry.Time / 1000;
				Nanosecond = true;
			}

			sprintf_s( TimeEntryName, "%s (%lli%s)", TimeEntry.Name.String().c_str(), Time, Nanosecond ? "ns" : ( Microsecond ? "us" : "ms" ) );
			const float BarTime = TimeEntry.Time / 1000000 / BarWindow;
			const float BarAverage = Average / 1000000 / BarWindow;
			const float BarPeak = Peak / 1000000 / BarWindow;

			const float DepthOffset = static_cast<float>( TimeEntry.Depth * 5.0f );
			ImVec2 PositionA = ImVec2( DepthOffset, EntryIndex * SliceHeight );
			ImVec2 PositionB = ImVec2( DepthOffset + Size.x * BarTime, ( EntryIndex + 1 ) * SliceHeight );
			ImVec2 PositionC = ImVec2( DepthOffset + Size.x * BarAverage, ( EntryIndex + 1 ) * SliceHeight );
			ImVec2 PositionD = ImVec2( DepthOffset + Size.x * BarPeak, ( EntryIndex + 1 ) * SliceHeight );

			const int R = 255 - ( EntryIndex % 4 ) * 63;
			const int G = ( EntryIndex % 8 ) * 31;
			const int B = ( EntryIndex % 64 ) * 3;
			DrawList->AddRectFilled( ImVec2( Position.x + PositionA.x, Position.y + PositionA.y ), ImVec2( Position.x + PositionB.x, Position.y + PositionB.y ), IM_COL32( R, G, B, 255 ) );
			// DrawList->AddRectFilled( ImVec2( Position.x + PositionA.x, Position.y + PositionA.y ), ImVec2( Position.x + PositionC.x, Position.y + PositionC.y ), IM_COL32( 255 - Alpha, 128, Alpha, 255 ) );
			// DrawList->AddRectFilled( ImVec2( Position.x + PositionD.x - 2.0f, Position.y + PositionA.y ), ImVec2( Position.x + PositionD.x, Position.y + PositionD.y ), IM_COL32( 255, 255, 255, 255 ) );
			DrawList->AddText( ImGui::GetFont(), SliceHeight, ImVec2( Position.x + PositionA.x, Position.y + PositionA.y ), IM_COL32( 255, 255, 255, 255 ), TimeEntryName );

			EntryIndex++;
		}

		DrawList->PopClipRect();
	}
}

void CProfiler::Display()
{
	if( !Enabled )
		return;

	ImGui::SetNextWindowPos( ImVec2( 0.0f, 25.0f ), ImGuiCond_Always );
	ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.3f ) ); // Transparent background
	if( ImGui::Begin( "Profiler", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
	{
		const auto& CurrentMemoryUsageBytes = GetMemoryUsageInBytes();

		if( TimeEntries.size() > 0 )
		{
			if( Enabled )
			{
				ImGui::Text( "Time Entries" );
				ImGui::Separator();
			}

			for( auto& TimeEntry : TimeEntries )
			{
				const bool Frametime = TimeEntry.first == "Frametime";
				const char* TimeEntryName = TimeEntry.first.String().c_str();
				RingBuffer<ProfileTimeEntry, TimeWindow>& Buffer = TimeEntry.second;

				float TimeValues[TimeWindow];
				for( size_t j = 0; j < TimeWindow; j++ )
				{
					TimeValues[j] = static_cast<float>( Buffer.Get( j ).Time ) / 1000000;
				}

				static const size_t AverageWindow = 32;
				float Average = 0.0f;
				int64_t Peak = 0;
				for( size_t j = 0; j < AverageWindow; j++ )
				{
					int64_t Time = Buffer.Get( Buffer.Offset( -static_cast<int>( j ) ) ).Time;
					Average += Time;

					if( Time > Peak )
					{
						Peak = Time;
					}
				}

				Average /= AverageWindow;

				Peak /= 1000000;
				Average /= 1000000;

				if( !Frametime )
				{
					// ImGui::Text( "%s: %ims (Peak: %ims)", TimeEntryName, static_cast<int64_t>( Average ), Peak );
				}
				else
				{
					if( Enabled )
					{
						ImGui::Text( "%s: %ims\nPeak: %ims\nFPS:%i\nFPS (Stutter/Low): %i", TimeEntryName, static_cast<int64_t>( Average ), Peak, static_cast<int64_t>( 1000.0f / Average ), static_cast<int64_t>( 1000.0f / static_cast<float>( Peak ) ) );
						ImGui::PushItemWidth( -1 );

						unsigned int ColorAverage = std::min( 255, std::max( 0, int( 2300.0f / Peak ) ) );
						unsigned int Red = 255 - ColorAverage;
						unsigned int Green = ColorAverage;

						ImGui::PushStyleColor( ImGuiCol_PlotHistogram, IM_COL32( Red, Green, 32, 255 ) );
						ImGui::PlotHistogram( "", TimeValues, static_cast<int>( TimeWindow ), static_cast<int>( Buffer.Offset() ), "Frametime", 0.0f, 33.3f, ImVec2( 500.0f, 100.0f ) );
						ImGui::PopStyleColor();
					}
					else
					{
						ImGui::Text( "%s: %ims (Peak: %ims)\nFPS:%i (Peak: %i)", TimeEntryName, static_cast<int64_t>( Average ), Peak, static_cast<int64_t>( 1000.0f / Average ), static_cast<int64_t>( 1000.0f / static_cast<float>( Peak ) ) );
					}
				}
			}

			
			ImGui::Text( "Memory Usage: %s (%s MB)", GetMemoryUsageAsString().c_str(), std::to_string( GetMemoryUsageInMegaBytes() ).c_str() );
			ImGui::Text( "Largest Allocation: %s", BytesToString( LargestAllocation ).c_str() );
			constexpr size_t MemoryWindow = 4096;
			static RingBuffer<size_t, MemoryWindow> MemoryBuffer;

			MemoryBuffer.Insert( GetMemoryUsageInMegaBytes() );

			static std::vector<float> MemoryValues;
			MemoryValues.clear();
			MemoryValues.resize( MemoryWindow );

			size_t MemoryCeiling = 0;
			for( size_t MemoryIndex = 0; MemoryIndex < MemoryWindow; MemoryIndex++ )
			{
				const auto& BufferValue = MemoryBuffer.Get( MemoryIndex );
				MemoryValues[MemoryIndex] = static_cast<float>( BufferValue );

				MemoryCeiling = std::max( MemoryCeiling, BufferValue );
			}

#ifdef _WIN32
			// Use the working set peak on Windows.
			MemoryCeiling = BytesToMegaBytes( GetMemoryCounters().PeakWorkingSetSize );
#endif

			ImGui::PushStyleColor( ImGuiCol_PlotHistogram, IM_COL32( 64, 128, 200, 255 ) );
			ImGui::PlotHistogram( "", MemoryValues.data(), static_cast<int>( MemoryWindow ), static_cast<int>( MemoryBuffer.Offset() ), "Memory Usage (MB)", 0.0f, static_cast<float>( MemoryCeiling ), ImVec2( 500.0f, 100.0f ) );
			ImGui::PopStyleColor();

			ImGui::Text( "" );
			
			PlotPerformance();
		}

		if( Enabled )
		{
			if( !MemoryCounters.empty() )
			{
				ImGui::Text( "\nMemory" );
				ImGui::Separator();

				// First pass, check how much memory we've accounted for.
				size_t AccountedBytes = MemoryStartBytes;
				for( const auto& Counter : MemoryCounters )
				{
					AccountedBytes += Counter.second;
				}

				const auto& Unaccounted = BytesToString( CurrentMemoryUsageBytes - AccountedBytes );
				ImGui::Text( "Unaccounted: %s", Unaccounted.c_str() );

				const auto& Accounted = BytesToString( AccountedBytes );
				ImGui::Text( "Accounted: %s", Accounted.c_str() );

				for( const auto& Counter : MemoryCounters )
				{
					const char* CounterName = Counter.first.String().c_str();
					const auto& CounterValue = BytesToString( Counter.second );
					ImGui::Text( "%s: %s", CounterName, CounterValue.c_str() );
				}

				if( MemoryCounters.size() > MemoryCounterGap )
				{
					MemoryCounterGap = MemoryCounters.size();
				}

				const size_t CounterDelta = MemoryCounterGap - MemoryCounters.size();
				for( size_t CounterIndex = 0; CounterIndex < CounterDelta; CounterIndex++ )
				{
					ImGui::Text( "" );
				}
			}

			if( TimeCounters.size() > 0 || TimeCountersFrame.size() > 0 )
			{
				ImGui::Text( "\nCounters" );
				ImGui::Separator();

				for( auto& TimeCounter : TimeCounters )
				{
					const char* TimeCounterName = TimeCounter.first.String().c_str();
					const uint64_t& TimeCounterValue = TimeCounter.second;
					ImGui::Text( "%s: %i", TimeCounterName, TimeCounterValue );
				}

				if( TimeCounters.size() > CounterGap )
				{
					CounterGap = TimeCounters.size();
				}

				size_t CounterDelta = CounterGap - TimeCounters.size();
				for( size_t CounterIndex = 0; CounterIndex < CounterDelta; CounterIndex++ )
				{
					ImGui::Text( "" );
				}

				for( auto& TimeCounter : TimeCountersFrame )
				{
					const char* TimeCounterName = TimeCounter.first.String().c_str();
					const uint64_t& TimeCounterValue = TimeCounter.second;
					ImGui::Text( "%s: %i", TimeCounterName, TimeCounterValue );
				}

				if( TimeCountersFrame.size() > CounterGapFrame )
				{
					CounterGapFrame = TimeCountersFrame.size();
				}

				size_t CounterFrameDelta = CounterGapFrame - TimeCountersFrame.size();
				for( size_t CounterIndex = 0; CounterIndex < CounterFrameDelta; CounterIndex++ )
				{
					ImGui::Text( "" );
				}
			}

			if( DebugMessages.size() > 0 )
			{
				ImGui::Text( "\nMessages" );
				ImGui::Separator();

				for( auto& DebugMessage : DebugMessages )
				{
					const char* DebugMessageName = DebugMessage.first.c_str();
					const char* DebugMessageBody = DebugMessage.second.c_str();
					ImGui::Text( "%s: %s", DebugMessageName, DebugMessageBody );
				}
			}

			auto& InputInterface = CInputLocator::Get();
			if( auto* Input = dynamic_cast<CInput*>( &InputInterface ) )
			{
				ImGui::Separator();

				auto* GamepadButtons = Input->GetGamepad();
				for( EGamepadType Index = 0; Index < MaximumGamepadButtons; Index++ )
				{
					const auto& Button = GamepadButtons[Index];
					if( Button.Action == EAction::Press )
					{
						ImGui::Text( "%s", GamepadLabels[Index] );
					}
				}

				auto* Keys = Input->GetKeys();
				for( EKeyType Index = 0; Index < MaximumKeyboardInputs; Index++ )
				{
					const auto& Key = Keys[Index];
					if( Key.Action == EAction::Press )
					{
						ImGui::Text( "Key %i", Index );
					}
				}

				auto* Mouse = Input->GetMouse();
				for( EMouseType Index = 0; Index < MaximumMouseButtons; Index++ )
				{
					const auto& Button = Mouse[Index];
					if( Button.Action == EAction::Press )
					{
						ImGui::Text( "%s", MouseLabels[Index] );
					}
				}
			}
		}

		ImGui::End();
	}
	ImGui::PopStyleColor();
}

void CProfiler::Clear()
{
	// TimeEntries.clear();
	// TimeCounters.clear();
	DebugMessages.clear();
}

void CProfiler::ClearFrame()
{
	TimeCountersFrame.clear();
}

bool CProfiler::IsEnabled() const
{
	return Enabled;
}

void CProfiler::SetEnabled( const bool EnabledIn )
{
	Enabled = EnabledIn;
}

size_t CProfiler::GetMemoryUsageInBytes()
{	
#ifdef _WIN32
	return GetMemoryUsageWorkingSet();
#else
	return MemoryUsageBytes;
#endif
}

size_t CProfiler::GetMemoryUsageInKiloBytes()
{
	return BytesToKiloBytes( GetMemoryUsageInBytes() );
}

size_t CProfiler::GetMemoryUsageInMegaBytes()
{
	return BytesToMegaBytes( GetMemoryUsageInBytes() );
}

size_t CProfiler::GetMemoryUsageInGigaBytes()
{
	return BytesToGigaBytes( GetMemoryUsageInBytes() );
}

std::string CProfiler::GetMemoryUsageAsString()
{
	return BytesToString( GetMemoryUsageInBytes() );
}

std::atomic<size_t> TimerScope::Depth = 0;
TimerScope::TimerScope( const NameSymbol& ScopeNameIn, bool TextOnlyIn )
{
	Depth++;
	ScopeName = ScopeNameIn;
	TextOnly = TextOnlyIn;
	StartTime = std::chrono::steady_clock::now();
}

TimerScope::TimerScope( const NameSymbol& ScopeNameIn, const uint64_t& Milliseconds )
{
	Depth++;
	ScopeName = ScopeNameIn;
	TextOnly = false;
	StartTime = std::chrono::steady_clock::now();
	StartTime = std::chrono::steady_clock::time_point( StartTime - std::chrono::milliseconds( Milliseconds ) );
}

TimerScope::~TimerScope()
{
	std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
	const auto DeltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>( EndTime - StartTime ).count();
	if( TextOnly )
	{
		Log::Event( "Scope %s took %ims\n", ScopeName.String().c_str(), DeltaTime );
	}
	else
	{
		const auto StartTimeValue = std::chrono::duration_cast<std::chrono::nanoseconds>( StartTime.time_since_epoch() ).count();
		CProfiler::Get().AddTimeEntry( ProfileTimeEntry( ScopeName, int64_t( DeltaTime ), int64_t( StartTimeValue ), Depth ) );
	}

	Depth--;
}

void TimerScope::Submit( const NameSymbol& ScopeNameIn, const std::chrono::steady_clock::time_point& StartTime, const uint64_t& Delta )
{
	const auto Time = std::chrono::milliseconds( Delta );
	const auto DeltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>( Time ).count();

	const auto StartTimeValue = std::chrono::duration_cast<std::chrono::nanoseconds>( StartTime.time_since_epoch() ).count();
	CProfiler::Get().AddTimeEntry( ProfileTimeEntry( ScopeNameIn, int64_t( DeltaTime ), int64_t( StartTimeValue ), Depth + 1 ) );
}

#undef ProfileMemory
ProfileMemory::ProfileMemory( const NameSymbol& ScopeNameIn, const bool& ClearPrevious )
{
	ScopeName = ScopeNameIn;
	MemoryStartSize = CProfiler::GetMemoryUsageInBytes();
	Clear = ClearPrevious;
}

ProfileMemory::~ProfileMemory()
{
	if( Clear )
	{
		CProfiler::Get().ClearMemoryEntry( ScopeName );
	}

	const auto Difference = CProfiler::GetMemoryUsageInBytes() - MemoryStartSize;
	if( Difference > 0 )
	{
		CProfiler::Get().AddMemoryEntry( ScopeName, Difference );
	}
}

Timer::Timer( bool UpdateOnGetElapsed )
{
	IsRunning = false;
	this->UpdatedOnGetElapsed = UpdateOnGetElapsed;
}

Timer::~Timer()
{

}

void Timer::Start()
{
	Start( std::chrono::milliseconds( 0 ) );
}

void Timer::Start( const uint64_t& Offset )
{
	Start( std::chrono::milliseconds( Offset ) );
}

void Timer::Start( const std::chrono::milliseconds& Offset )
{
	StartTime = std::chrono::steady_clock::now() - Offset;
	IsRunning = true;
}

void Timer::Stop()
{
	StopTime = std::chrono::steady_clock::now();
	IsRunning = false;
}

bool Timer::Enabled() const
{
	return IsRunning;
}

int64_t Timer::GetElapsedTimeNanoseconds()
{
	if( IsRunning )
	{
		std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
		const auto DeltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>( EndTime - StartTime ).count();

		if( UpdatedOnGetElapsed )
		{
			StartTime = std::chrono::steady_clock::now();
		}

		return DeltaTime;
	}
	else
	{
		const auto DeltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>( StopTime - StartTime ).count();

		if( UpdatedOnGetElapsed )
		{
			StartTime = std::chrono::steady_clock::now();
		}

		return DeltaTime;
	}
}

int64_t Timer::GetElapsedTimeMicroseconds()
{
	if( IsRunning )
	{
		std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
		const auto DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>( EndTime - StartTime ).count();

		if( UpdatedOnGetElapsed )
		{
			StartTime = std::chrono::steady_clock::now();
		}

		return DeltaTime;
	}
	else
	{
		const auto DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>( StopTime - StartTime ).count();

		if( UpdatedOnGetElapsed )
		{
			StartTime = std::chrono::steady_clock::now();
		}

		return DeltaTime;
	}
}

int64_t Timer::GetElapsedTimeMilliseconds()
{
	if( IsRunning )
	{
		std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
		const auto DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>( EndTime - StartTime ).count();

		if( UpdatedOnGetElapsed )
		{
			StartTime = std::chrono::steady_clock::now();
		}

		return DeltaTime;
	}
	else
	{
		const auto DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>( StopTime - StartTime ).count();

		if( UpdatedOnGetElapsed )
		{
			StartTime = std::chrono::steady_clock::now();
		}

		return DeltaTime;
	}
}

double Timer::GetElapsedTimeSeconds()
{
	const int64_t Time = GetElapsedTimeMilliseconds();
	return static_cast<double>( Time ) / 1000.0;
}

std::chrono::steady_clock::time_point Timer::GetStartTime()
{
	return StartTime;
}

std::chrono::steady_clock::time_point Timer::GetStopTime()
{
	return StopTime;
}

bool ProfileTimeEntry::operator<( const ProfileTimeEntry& Entry ) const
{
	Log::Event( "Comparing\n" );
	return StartTime < Entry.StartTime;
}
