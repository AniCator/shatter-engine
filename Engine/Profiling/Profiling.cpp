// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Profiling.h"
#include "Logging.h"

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
		CRingBuffer<int64_t, TimeWindow> Buffer;
		Buffer.Insert( TimeEntry.Time );
		TimeEntries.insert_or_assign( TimeEntry.Name, Buffer );
	}
	else
	{
		Iterator->second.Insert( TimeEntry.Time );
	}
}

void CProfiler::AddCounterEntry( const char* NameIn, int TimeIn )
{
	if( !Enabled )
		return;

	auto Iterator = TimeCounters.find( NameIn );
	if( Iterator == TimeCounters.end() )
	{
		TimeCounters.insert_or_assign( NameIn, TimeIn );
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
			TimeCountersFrame.insert_or_assign( TimeEntry.Name, TimeEntry.Time );
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
			TimeCounters.insert_or_assign( TimeEntry.Name, TimeEntry.Time );
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

		int EntryIndex = 0;
		const int SliceAlpha = 255 / TimeEntries.size();
		static const float BarWindow = 1.0f / 24.0f * 1000.0f;

		for( auto& TimeEntry : TimeEntries )
		{
			const char* TimeEntryName = TimeEntry.first.c_str();
			CRingBuffer<int64_t, TimeWindow>& Buffer = TimeEntry.second;
			const int Alpha = SliceAlpha * EntryIndex;

			static const size_t AverageWindow = 128;
			float Average = 0.0f;
			int64_t Peak = 0;
			for( size_t j = 0; j < AverageWindow; j++ )
			{
				int64_t Time = Buffer.Get( Buffer.Offset( -static_cast<int>( j ) ) );
				Average += Time;

				if( Time > Peak )
				{
					Peak = Time;
				}
			}

			Average /= AverageWindow;

			const int64_t Time = Buffer.Get( Buffer.Offset( -1 ) );
			const float BarTime = Time / BarWindow;
			const float BarAverage = Average / BarWindow;
			const float BarPeak = Peak / BarWindow;

			ImVec2 PositionA = ImVec2( 0.0f, EntryIndex * SliceHeight );
			ImVec2 PositionB = ImVec2( Size.x * BarTime, ( EntryIndex + 1 ) * SliceHeight );
			ImVec2 PositionC = ImVec2( Size.x * BarAverage, ( EntryIndex + 1 ) * SliceHeight );
			ImVec2 PositionD = ImVec2( Size.x * BarPeak, ( EntryIndex + 1 ) * SliceHeight );

			DrawList->AddRectFilled( ImVec2( Position.x + PositionA.x, Position.y + PositionA.y ), ImVec2( Position.x + PositionB.x, Position.y + PositionB.y ), IM_COL32( 255 - Alpha, 0, Alpha, 255 ) );
			DrawList->AddRectFilled( ImVec2( Position.x + PositionA.x, Position.y + PositionA.y ), ImVec2( Position.x + PositionC.x, Position.y + PositionC.y ), IM_COL32( 255 - Alpha, 128, Alpha, 255 ) );
			DrawList->AddRectFilled( ImVec2( Position.x + PositionD.x - 2.0f, Position.y + PositionA.y ), ImVec2( Position.x + PositionD.x, Position.y + PositionD.y ), IM_COL32( 255, 255, 255, 255 ) );
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
				const char* TimeEntryName = TimeEntry.first.c_str();
				CRingBuffer<int64_t, TimeWindow>& Buffer = TimeEntry.second;

				float TimeValues[TimeWindow];
				for( size_t j = 0; j < TimeWindow; j++ )
				{
					TimeValues[j] = static_cast<float>( Buffer.Get( j ) );
				}

				static const size_t AverageWindow = 32;
				float Average = 0.0f;
				int64_t Peak = 0;
				for( size_t j = 0; j < AverageWindow; j++ )
				{
					int64_t Time = Buffer.Get( Buffer.Offset( -static_cast<int>( j ) ) );
					Average += Time;

					if( Time > Peak )
					{
						Peak = Time;
					}
				}

				Average /= AverageWindow;

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
					const char* TimeCounterName = TimeCounter.first.c_str();
					const uint64_t& TimeCounterValue = TimeCounter.second;
					ImGui::Text( "%s: %i", TimeCounterName, TimeCounterValue );
				}

				for( auto& TimeCounter : TimeCountersFrame )
				{
					const char* TimeCounterName = TimeCounter.first.c_str();
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

CTimerScope::CTimerScope( const char* ScopeNameIn, bool TextOnlyIn )
{
	strcpy_s( ScopeName, ScopeNameIn );
	TextOnly = TextOnlyIn;
	StartTime = std::chrono::steady_clock::now();
};

CTimerScope::~CTimerScope()
{
	std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
	const auto DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>( EndTime - StartTime ).count();
	if( TextOnly )
	{
		Log::Event( "Scope %s took %ims\n", ScopeName, DeltaTime );
	}
	else
	{
		CProfiler::Get().AddTimeEntry( FProfileTimeEntry( ScopeName, int64_t( DeltaTime ) ) );
	}
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
