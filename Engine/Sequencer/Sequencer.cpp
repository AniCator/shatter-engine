// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sequencer.h"

#include <string>
#include <algorithm>

#include <Engine/Audio/Sound.h>
#include <Game/Game.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/File.h>
#include <Engine/World/World.h>

#include <ThirdParty/imgui-1.70/imgui.h>

static const Timecode Timebase = 96;

enum class ESequenceStatus : uint8_t
{
	Stopped = 0,
	Paused,
	Playing
};

struct FTrackEvent
{
	virtual void Evaluate( const Timecode& Marker ) = 0;
	virtual void Execute() = 0;
	virtual void Reset() = 0;
	virtual void Context() = 0;

	virtual const char* GetName() = 0;

	Timecode Start = 0;
	Timecode Length = 0;
};

struct FEventAudio : FTrackEvent
{
	virtual void Evaluate( const Timecode& Marker ) override
	{
		if( !Triggered && Marker >= Start && Marker < ( Start + Length ) )
		{
			Execute();
			Triggered = true;
		}
		else
		{
			if( Triggered && Marker > ( Start + Length ) )
			{
				if( Sound && Sound->Playing() )
				{
					Sound->Stop( FadeOut );
				}

				Triggered = false;
			}
		}
	}

	virtual void Execute() override
	{
		if( !Sound )
			return;

		if( Sound )
		{
			Sound->Volume( Volume );
			Sound->Start( FadeIn );
		}
	}

	virtual void Reset() override
	{
		Triggered = false;

		if( Sound && Sound->Playing() )
		{
			Sound->Stop();
		}
	}

	virtual void Context() override
	{
		ImGui::Text( "Test audio event" );
		ImGui::Text( "%s", Name.length() > 0 && Sound != nullptr ? Name.c_str() : "no sound selected" );
		
		ImGui::Separator();
		int InputLength = (int) Length / Timebase;
		if( ImGui::InputInt( "Length", &InputLength ) )
		{
			Length = (Timecode) InputLength * Timebase;
		}

		ImGui::InputFloat( "Volume", &Volume, 1.0f, 10.0f, "%.0f" );
		ImGui::InputFloat( "Fade-In", &FadeIn, 1.0f, 10.0f, "%.0f" );
		ImGui::InputFloat( "Fade-Out", &FadeOut, 1.0f, 10.0f, "%.0f" );

		ImGui::Separator();

		auto& Assets = CAssets::Get();
		auto& Sounds = Assets.GetSounds();
		for( auto& Pair : Sounds )
		{
			if( ImGui::Selectable( Pair.first.c_str() ) )
			{
				Name = Pair.first;
				Sound = Pair.second;
			}

			ImGui::Separator();
		}
	}

	virtual const char* GetName()
	{
		return Name.c_str();
	}

	bool Triggered = false;
	std::string Name = std::string();
	CSound* Sound = nullptr;

	float FadeIn = 0.0f;
	float FadeOut = 0.1f;
	float Volume = 100.0f;
};

struct FTrack
{
	void AddEvent( FTrackEvent* Event )
	{
		Events.emplace_back( Event );
	}

	void Evaluate( const Timecode& Marker )
	{
		for( auto& Event : Events )
		{
			if( Event )
			{
				Event->Evaluate( Marker );
			}
		}
	}

	void Reset()
	{
		for( auto& Event : Events )
		{
			if( Event )
			{
				Event->Reset();
			}
		}
	}

	Timecode Start;
	Timecode Length;

	std::vector<FTrackEvent*> Events;
};

class CTimeline
{
public:
	CTimeline( Timecode Start = 0, Timecode End = 0 )
	{
		StartMarker = Start;
		EndMarker = End;
		Marker = 0;

		DrawTimeline = false;
	}

	~CTimeline()
	{

	}

	bool Load( const char* FileLocation )
	{
		Location = FileLocation;
		CFile File( Location.c_str() );
		if( File.Exists() )
		{
			File.Load();

			return true;
		}

		return false;
	}

	void Save( const char* FileLocation = nullptr )
	{
		const char* SaveLocation = FileLocation ? FileLocation : Location.c_str();
		if( SaveLocation )
		{
			CFile File( SaveLocation );
		}
	}

	void Play()
	{
		if( Status != ESequenceStatus::Paused )
		{
			for( auto& Track : Tracks )
			{
				Track.Reset();
			}
		}

		Status = ESequenceStatus::Playing;
	}

	void Pause()
	{
		Status = ESequenceStatus::Paused;
	}

	void Stop()
	{
		Status = ESequenceStatus::Stopped;
		Marker = StartMarker;

		for( auto& Track : Tracks )
		{
			Track.Reset();
		}
	}

	bool Playing() const
	{
		return Status == ESequenceStatus::Playing;
	}

	bool Stopped() const
	{
		return Status == ESequenceStatus::Stopped;
	}

	void Step()
	{
		if( Status < ESequenceStatus::Playing )
			return;

		// Run all the timeline events that are associated with the current marker.
		for( auto& Track : Tracks )
		{
			Track.Evaluate( Marker );
		}

		// Increment the timeline step.
		Marker++;

		// Stop the timeline playback if we have crossed the end marker.
		if( Marker >= EndMarker )
		{
			Stop();
		}
	}

	void GoTo( const Timecode MarkerLocation )
	{
		Marker = MarkerLocation;
	}

	Timecode CurrentMarker() const
	{
		return Marker;
	}

	Timecode Size() const
	{
		return StartMarker - EndMarker;
	}

	float Time() const
	{
		return static_cast<float>( static_cast<double>( Marker ) / static_cast<double>( Timebase ) );
	}

	float Length() const
	{
		return static_cast<float>( static_cast<double>( EndMarker - StartMarker ) / static_cast<double>( Timebase ) );
	}

	void Frame()
	{
		if( DrawTimeline )
		{
			if( ImGui::Begin( "Timeline", &DrawTimeline, ImVec2( 1000.0f, 500.0f ) ) )
			{
				static FTrack* ActiveTrack = nullptr;
				static int SequenceLengthSeconds = 2;

				if( ImGui::Button( "Play" ) )
				{
					Play();
				}

				ImGui::SameLine();

				if( ImGui::Button( "Pause" ) )
				{
					Pause();
				}

				ImGui::SameLine();

				if( ImGui::Button( "Stop" ) )
				{
					Stop();
				}

				ImGui::SameLine();

				if( ImGui::Button( "Create Track" ) )
				{
					CreateTrack();
					ActiveTrack = nullptr;
				}

				ImGui::SameLine();

				if( ImGui::Button( "Create Event" ) )
				{
					if( ActiveTrack && Tracks.size() > 0 )
					{
						FEventAudio* Event = new FEventAudio();
						Event->Start = Marker;
						Event->Length = SequenceLengthSeconds * Timebase;
						ActiveTrack->AddEvent( Event );

						Marker += Event->Length;

						auto EventCode = Event->Start + Event->Length;
						auto TrackCode = ActiveTrack->Start + ActiveTrack->Length;
						if( EventCode > TrackCode )
						{
							ActiveTrack->Length = EventCode - ActiveTrack->Start;
						}

						Timecode AdjustedEnd = ActiveTrack->Start + ActiveTrack->Length;
						if( AdjustedEnd > EndMarker )
						{
							EndMarker = AdjustedEnd;
						}
					}
				}

				ImGui::Separator();

				int StartLocation = StartMarker / Timebase;
				int EndLocation = EndMarker / Timebase;
				if( ImGui::InputInt( "Start", &StartLocation ) )
				{
					StartMarker = StartLocation * Timebase;
				}

				if( ImGui::InputInt( "End", &EndLocation ) )
				{
					EndMarker = EndLocation * Timebase;
				}

				ImGui::Separator();
				ImGui::Separator();

				auto CursorOffset = 100.0f;
				auto CursorPosition = ImGui::GetCursorPos();
				CursorPosition.x += CursorOffset;
				size_t WidthDelta = static_cast<size_t>( 1000.0f / 30 );
				size_t Bars = ( ( EndMarker - StartMarker ) / Timebase );
				for( size_t BarIndex = 0; BarIndex < Bars; BarIndex++ )
				{
					auto BarPosition = CursorPosition;
					BarPosition.x += BarIndex * WidthDelta;
					ImGui::SetCursorPos( BarPosition );

					ImGui::Text( "%zi", BarIndex );
				}

				ImGui::Separator();

				CursorPosition = ImGui::GetCursorPos();
				auto ScreenPosition = ImGui::GetCursorScreenPos();

				struct FEventTrackDrag
				{
					bool Up;
					size_t TrackIndex;
					FTrackEvent* Event;
				};
				std::vector<FEventTrackDrag> DragEvents;
				DragEvents.reserve( 2 );

				size_t GlobalIndex = 0;
				size_t TrackIndex = 0;
				for( auto& Track : Tracks )
				{
					auto TrackPosition = ImGui::GetCursorPos();
					TrackPosition.x += ( static_cast<float>( Track.Start ) / static_cast<float>( Timebase ) ) * WidthDelta;
					ImGui::SetCursorPos( TrackPosition );

					const float Hue = TrackIndex * 0.05f;
					const float Value = &Track == ActiveTrack ? 0.4f : 0.2f;
					ImGui::PushStyleColor( ImGuiCol_ChildBg, static_cast<ImVec4>( ImColor::HSV( Hue, 0.3f, Value ) ) );

					char TrackName[128];
					sprintf_s( TrackName, "Track %zi", TrackIndex );
					ImGui::BeginChild( TrackName, ImVec2( CursorOffset + ( static_cast<float>( Track.Length ) / static_cast<float>( Timebase ) ) * WidthDelta, 30.0f ), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );

					TrackPosition = ImGui::GetCursorPos();

					if( ImGui::Selectable( TrackName, false, 0, ImVec2( CursorOffset - 5.0f, 30.0f ) ) )
					{
						ActiveTrack = &Track;
					}

					TrackPosition.x += CursorOffset;
					ImGui::SetCursorPosX( TrackPosition.x );

					size_t EventIndex = 0;
					for( auto& Event : Track.Events )
					{
						auto EventPosition = TrackPosition;
						EventPosition.x += ( static_cast<float>( Event->Start ) / static_cast<float>( Timebase ) ) * WidthDelta;
						ImGui::SetCursorPos( EventPosition );

						float Hue = 270.0f - ( GlobalIndex * 0.2f );
						ImGui::PushStyleColor( ImGuiCol_Button, static_cast<ImVec4>( ImColor::HSV( Hue, 0.6f, 0.6f ) ) );
						ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 0.0f );
						ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );

						char EventName[128];
						sprintf_s( EventName, "%s (Start: %lli End: %lli Length: %llis)##eb%zi", Event->GetName(), Event->Start, Event->Start + Event->Length, Event->Length / Timebase, GlobalIndex );
						ImGui::Button( EventName, ImVec2( ( Event->Length / Timebase ) * WidthDelta, 30.0f ) );
						if( ImGui::IsItemHovered() )
						{
							ImGui::SetTooltip( EventName );
						}

						char ContextName[128];
						sprintf_s( ContextName, "ec##%zi", EventIndex );
						if( ImGui::BeginPopupContextItem( ContextName ) )
						{
							Event->Context();
							ImGui::EndPopup();
						}

						if( ImGui::IsItemClicked() )
						{
							ActiveTrack = &Track;
							Event->Execute();
						}

						ImVec2 Drag = ImVec2();

						if( ImGui::IsItemHovered() && ImGui::GetIO().MouseDown[0] )
						{
							auto Delta = ImGui::GetMouseDragDelta( 0, 0.0f );
							Drag.x += Delta.x;
							Drag.y += Delta.y;
							ImGui::ResetMouseDragDelta( 0 );

							Marker = Event->Start;
						}

						auto NewEventStart = Event->Start + static_cast<Timecode>( Drag.x / WidthDelta * Timebase );
						if( fabs( Drag.x ) > 0.001f && NewEventStart > Track.Start && NewEventStart < ( Track.Start + Track.Length ) )
						{
							Event->Start = NewEventStart;

							auto EventCode = Event->Start + Event->Length;
							auto TrackCode = Track.Start + Track.Length;
							if( EventCode > TrackCode )
							{
								Track.Length = EventCode - Track.Start;
							}
						}

						if( fabs( Drag.y ) > 1.0f && ImGui::IsItemActive() ) // 
						{
							const bool Up = Drag.y < 0.0f;
							FEventTrackDrag DragEvent;
							DragEvent.Up = Up;
							DragEvent.TrackIndex = TrackIndex;
							DragEvent.Event = Event;

							DragEvents.emplace_back( DragEvent );
						}

						ImGui::PopStyleVar();
						ImGui::PopStyleVar();
						ImGui::PopStyleColor();

						EventIndex++;
						GlobalIndex++;
					}

					TrackIndex++;

					ImGui::EndChild();

					ImGui::PopStyleColor();
				}

				auto DrawList = ImGui::GetWindowDrawList();
				if( DrawList )
				{
					auto VerticalOffset = 27.5f;
					ScreenPosition.x += CursorOffset;

					auto MarkerPosition = ScreenPosition;
					MarkerPosition.x += ( static_cast<float>( Marker ) / static_cast<float>( Timebase ) ) * WidthDelta;
					auto LineEnd = MarkerPosition;
					LineEnd.y += Tracks.size() * VerticalOffset;
					DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32_WHITE );

					MarkerPosition = ScreenPosition;
					MarkerPosition.x += ( StartMarker / Timebase ) * WidthDelta;
					LineEnd = MarkerPosition;
					LineEnd.y += Tracks.size() * VerticalOffset;
					DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32_BLACK );

					MarkerPosition = ScreenPosition;
					MarkerPosition.x += ( EndMarker / Timebase ) * WidthDelta;
					LineEnd = MarkerPosition;
					LineEnd.y += Tracks.size() * VerticalOffset;
					DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32_BLACK );
				}

				bool DragEventOccured = false;
				for( auto& DragEvent : DragEvents )
				{
					size_t NewTrackIndex = std::min( Tracks.size() - 1, std::max( size_t( 0 ), DragEvent.Up ? ( DragEvent.TrackIndex == 0 ? 0 : DragEvent.TrackIndex - 1 ) : DragEvent.TrackIndex + 1 ) );
					if( NewTrackIndex != DragEvent.TrackIndex )
					{
						auto Iterator = std::find( Tracks[DragEvent.TrackIndex].Events.begin(), Tracks[DragEvent.TrackIndex].Events.end(), DragEvent.Event );
						if( Iterator != Tracks[DragEvent.TrackIndex].Events.end() )
						{
							Tracks[DragEvent.TrackIndex].Events.erase( Iterator );
							Tracks[NewTrackIndex].Events.emplace_back( DragEvent.Event );
							DragEventOccured = true;
							ActiveTrack = &Tracks[NewTrackIndex];
						}
					}

				}

				if( DragEventOccured )
				{
					for( auto& Track : Tracks )
					{
						for( auto Event : Track.Events )
						{
							auto EventCode = Event->Start + Event->Length;
							auto TrackCode = Track.Start + Track.Length;
							if( EventCode > TrackCode )
							{
								Track.Length = EventCode - Track.Start;
							}
						}
					}
				}
			}

			ImGui::End();
		}

		static float PreviousTime = 0.0f;
		auto Time = GameLayersInstance->GetCurrentTime();
		auto DeltaTime = Time - PreviousTime;
		size_t Steps = DeltaTime / ( 1.0f / (float) Timebase );
		for( size_t StepIndex = 0; StepIndex < Steps; StepIndex++ )
		{
			Step();
		}

		PreviousTime = Time;
	}

	void Draw()
	{
		DrawTimeline = true;
	}

	void CreateTrack()
	{
		FTrack Track;
		Track.Start = StartMarker;
		Track.Length = EndMarker - StartMarker;

		Tracks.emplace_back( Track );
	}

private:
	bool DrawTimeline;

private:
	std::string Location;
	Timecode Marker;

	Timecode StartMarker;
	Timecode EndMarker;

	ESequenceStatus Status;

	std::vector<FTrack> Tracks;
};

CSequence::CSequence()
{
	Timeline = new CTimeline();
}

CSequence::~CSequence()
{
	if( Timeline )
	{
		delete Timeline;
		Timeline = nullptr;
	}
}

bool CSequence::Load( const char* FileLocation )
{
	if( Timeline )
	{
		return Timeline->Load( FileLocation );
	}

	return false;
}

void CSequence::Save( const char* FileLocation )
{
	if( Timeline )
	{
		Timeline->Save( FileLocation );
	}
}

void CSequence::Play()
{
	if( Timeline )
	{
		Timeline->Play();
	}
}

void CSequence::Pause()
{
	if( Timeline )
	{
		Timeline->Pause();
	}
}

void CSequence::Stop()
{
	if( Timeline )
	{
		Timeline->Stop();
	}
}

bool CSequence::Playing() const
{
	if( Timeline )
	{
		return Timeline->Playing();
	}

	return false;
}

bool CSequence::Stopped() const
{
	if( Timeline )
	{
		return Timeline->Stopped();
	}

	return true;
}

void CSequence::Step()
{
	if( Timeline )
	{
		Timeline->Step();
	}
}

void CSequence::GoTo( const Timecode Marker )
{
	if( Timeline )
	{
		Timeline->GoTo( Marker );
	}
}

Timecode CSequence::CurrentMarker() const
{
	if( Timeline )
	{
		return Timeline->CurrentMarker();
	}

	return 0;
}

Timecode CSequence::Size() const
{
	if( Timeline )
	{
		return Timeline->Size();
	}

	return 0;
}

float CSequence::Time() const
{
	if( Timeline )
	{
		return Timeline->Time();
	}

	return 0.0f;
}

float CSequence::Length() const
{
	if( Timeline )
	{
		return Timeline->Length();
	}

	return 0.0f;
}

void CSequence::Frame()
{
	if( Timeline )
	{
		Timeline->Frame();
	}
}

void CSequence::Draw()
{
	if( Timeline )
	{
		Timeline->Draw();
	}
}
