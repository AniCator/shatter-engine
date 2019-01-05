// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Profiling.h"
#include "Logging.h"

#include <ThirdParty/imgui-1.52/imgui.h>

CProfileVisualisation::CProfileVisualisation()
{
#ifdef ProfileBuild
	Enabled = true;
#else
	Enabled = false;
#endif
}

CProfileVisualisation::~CProfileVisualisation()
{
	Clear();
}

void CProfileVisualisation::AddTimeEntry( FProfileTimeEntry& TimeEntry )
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

void CProfileVisualisation::AddCounterEntry( const char* NameIn, int TimeIn )
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

void CProfileVisualisation::AddCounterEntry( FProfileTimeEntry& TimeEntry, const bool PerFrame )
{
	if( !Enabled )
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

void CProfileVisualisation::AddDebugMessage( const char* NameIn, const char* Body )
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

void CProfileVisualisation::Display()
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

			int EntryIndex = 0;
			for( auto& TimeEntry : TimeEntries )
			{
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

				if( EntryIndex > 0 )
				{
					ImGui::Text( "%s: %ims (Peak: %ims)", TimeEntryName, static_cast<int64_t>( Average ), Peak );
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
						ImGui::Text( "%s: %ims (Peak: %ims)\n", TimeEntryName, static_cast<int64_t>( Average ), Peak );
					}
				}

				EntryIndex++;
			}
		}

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

		ImGui::End();
	}
	ImGui::PopStyleColor();

	// Clear();
	TimeCountersFrame.clear();
}

void CProfileVisualisation::Clear()
{
	// TimeEntries.clear();
	TimeCounters.clear();
	// DebugMessages.clear();
}

bool CProfileVisualisation::IsEnabled() const
{
	return Enabled;
}

void CProfileVisualisation::SetEnabled( const bool EnabledIn )
{
	Enabled = EnabledIn;
}

CTimerScope::CTimerScope( const char* ScopeNameIn, bool TextOnlyIn )
{
	strcpy_s( ScopeName, ScopeNameIn );
	StartTime = std::chrono::steady_clock::now();
	TextOnly = TextOnlyIn;
};

CTimerScope::~CTimerScope()
{
	std::chrono::steady_clock::time_point EndTime = std::chrono::steady_clock::now();
	const auto DeltaTime = std::chrono::duration_cast<std::chrono::milliseconds>( EndTime - StartTime ).count();
	if( TextOnly )
	{
		Log::Event( "-- %s - %ims --\n\n", ScopeName, DeltaTime );
	}
	else
	{
		CProfileVisualisation::GetInstance().AddTimeEntry( FProfileTimeEntry( ScopeName, int64_t( DeltaTime ) ) );
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
