// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Profiling.h"
#include "Logging.h"

#include <algorithm>

#include <ThirdParty/imgui-1.70/imgui.h>

CProfiler::CProfiler()
{
#ifdef ProfileBuild
	Enabled = true;
#else
	Enabled = false;
#endif
}

CProfiler::~CProfiler()
{
	Clear();
}

void CProfiler::AddTimeEntry( FProfileTimeEntry& TimeEntry )
{
	auto Iterator = TimeEntries.find( TimeEntry.Name );
	if( Iterator == TimeEntries.end() )
	{
		CRingBuffer<FProfileTimeEntry, TimeWindow> Buffer;
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
	if( !Enabled )
		return;

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

void CProfiler::AddCounterEntry( FProfileTimeEntry& TimeEntry, const bool PerFrame )
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
			Iterator->second += TimeEntry.Time;
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
			Iterator->second += TimeEntry.Time;
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
		std::vector<CRingBuffer<FProfileTimeEntry, TimeWindow>*> PlottableEntries;
		for( auto& Pair : TimeEntries )
		{
			auto& Buffer = Pair.second;
			const auto& TimeEntry = Buffer.Get( Buffer.Offset( -1 ) );
			const auto Difference = CurrentTime - TimeEntry.StartTime;
			if( Difference > 1000000000 )
				continue;

			PlottableEntries.emplace_back( &Buffer );
		}

		auto SortEntries = [&] ( CRingBuffer<FProfileTimeEntry, TimeWindow>* A, CRingBuffer<FProfileTimeEntry, TimeWindow>* B ) {
			const auto& TimeEntryA = A->Get( A->Offset( -1 ) );
			const auto& TimeEntryB = B->Get( B->Offset( -1 ) );
			return ExclusiveComparison( TimeEntryA.StartTime, TimeEntryB.StartTime );
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
			int64_t Time = TimeEntry.Time / 1000000;
			if( Time == 0 )
			{
				Time = TimeEntry.Time / 100000;
				Microsecond = true;
			}

			if( Time == 0 )
				continue;

			sprintf_s( TimeEntryName, "%s (%lli%s)", TimeEntry.Name.String().c_str(), Time, Microsecond ? "us" : "ms" );
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
	ImGui::SetNextWindowPos( ImVec2( 0.0f, 25.0f ), ImGuiCond_Always );
	ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.3f ) ); // Transparent background
	if( ImGui::Begin( "Profiler", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
	{
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
				CRingBuffer<FProfileTimeEntry, TimeWindow>& Buffer = TimeEntry.second;

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
						ImGui::Text( "%s: %ims\nPeak: %ims\nFPS:%i\nFPS (Lowest): %i", TimeEntryName, static_cast<int64_t>( Average ), Peak, static_cast<int64_t>( 1000.0f / Average ), static_cast<int64_t>( 1000.0f / static_cast<float>( Peak ) ) );
						ImGui::PushItemWidth( -1 );
						ImGui::PlotHistogram( "", TimeValues, static_cast<int>( TimeWindow ), static_cast<int>( Buffer.Offset() ), "Histogram (Frametime)", 0.0f, 33.3f, ImVec2( 500.0f, 100.0f ) );
						ImGui::Text( "" );
					}
					else
					{
						ImGui::Text( "%s: %ims (Peak: %ims)\nFPS:%i (Lowest: %i)", TimeEntryName, static_cast<int64_t>( Average ), Peak, static_cast<int64_t>( 1000.0f / Average ), static_cast<int64_t>( 1000.0f / static_cast<float>( Peak ) ) );
					}
				}
			}

			PlotPerformance();
		}

		if( Enabled )
		{
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

				for( auto& TimeCounter : TimeCountersFrame )
				{
					const char* TimeCounterName = TimeCounter.first.String().c_str();
					const uint64_t& TimeCounterValue = TimeCounter.second;
					ImGui::Text( "%s: %i", TimeCounterName, TimeCounterValue );
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

std::atomic<size_t> CTimerScope::Depth = 0;
CTimerScope::CTimerScope( const FName& ScopeNameIn, bool TextOnlyIn )
{
	Depth++;
	ScopeName = ScopeNameIn;
	TextOnly = TextOnlyIn;
	StartTime = std::chrono::steady_clock::now();
};

CTimerScope::~CTimerScope()
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
		CProfiler::Get().AddTimeEntry( FProfileTimeEntry( ScopeName, int64_t( DeltaTime ), int64_t( StartTimeValue ), Depth ) );
	}

	Depth--;
};

CTimer::CTimer( bool UpdateOnGetElapsed )
{
	IsRunning = false;
	this->UpdatedOnGetElapsed = UpdateOnGetElapsed;
}

CTimer::~CTimer()
{

}

void CTimer::Start()
{
	StartTime = std::chrono::steady_clock::now();
	IsRunning = true;
}

void CTimer::Stop()
{
	StopTime = std::chrono::steady_clock::now();
	IsRunning = false;
}

bool CTimer::Enabled() const
{
	return IsRunning;
}

int64_t CTimer::GetElapsedTimeMicroseconds()
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

int64_t CTimer::GetElapsedTimeMilliseconds()
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

double CTimer::GetElapsedTimeSeconds()
{
	const int64_t Time = GetElapsedTimeMilliseconds();
	return static_cast<double>( Time ) / 1000.0;
}

bool FProfileTimeEntry::operator<( const FProfileTimeEntry& Entry ) const
{
	Log::Event( "Comparing\n" );
	return StartTime < Entry.StartTime;
}
