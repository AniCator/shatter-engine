// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Sequencer.h"

#include <string>
#include <algorithm>

#include <Engine/Audio/Sound.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/DataString.h>
#include <Engine/World/World.h>

#include <Engine/Sequencer/Events/ImageEvent.h>

#include <Game/Game.h>

#include <ThirdParty/imgui-1.70/imgui.h>

static bool TimelineCamera = true;
static CCamera* ActiveTimelineCamera = nullptr;

// True if the mouse is held down while dragging on the timeline bar.
static bool Scrubbing = false;

double MarkerToTime( const Timecode& Marker )
{
	return StaticCast<double>( Marker ) / StaticCast<double>( Timebase );
	return StaticCast<double>( Marker ) / StaticCast<double>( Timebase );
}

double MarkerRangeToTime( const Timecode& StartMarker, const Timecode& EndMarker )
{
	return StaticCast<double>( EndMarker - StartMarker ) / StaticCast<double>( Timebase );
}

enum class ESequenceStatus : uint8_t
{
	Stopped = 0,
	Paused,
	Playing
};

struct FEventAudio : FTrackEvent
{
	virtual void Evaluate( const Timecode& Marker ) override
	{
		if( !Sound )
		{
			Sound = CAssets::Get().FindSound( Name );
			return;
		}
		
		if( !Triggered && Marker >= Start && Marker < ( Start + Length ) )
		{
			Execute();
			Triggered = true;
		}
		else
		{
			if( Triggered && ( Marker > ( Start + Length ) || ( Marker < Start ) ) )
			{
				if( Sound->Playing() )
				{
					Sound->Stop( FadeOut );
				}

				Triggered = false;
			}

			if( Triggered )
			{
				if( Scrubbing )
				{
					if( !Sound->Playing() )
					{
						auto Information = Spatial::CreateUI();

						const auto RelativeTime = MarkerToTime( Offset );
						if( RelativeTime < FadeIn )
						{
							Information.FadeIn = FadeIn - RelativeTime;
						}

						Information.Volume = Volume;
						Information.Rate = Rate;
						Sound->Start( Information );
					}

					Sound->Offset( MarkerToTime( Offset ) );
				}
				else
				{
					if( Frozen() && Sound->Playing() )
					{
						Sound->Stop();
					}
				}
			}
		}
	}

	virtual void Execute() override
	{
		if( !Sound )
			return;

		if( !Sound->Playing() )
		{		
			auto Information = Spatial::CreateUI();

			const auto RelativeTime = MarkerToTime( Offset );
			if( RelativeTime < FadeIn )
			{
				Information.FadeIn = FadeIn - RelativeTime;
			}
			
			Information.Volume = Volume;
			Information.Rate = Rate;
			Sound->Start( Information );
		}

		if( Scrubbing )
		{
			Sound->Offset( MarkerToTime( Offset ) );
		}
	}

	virtual void Reset() override
	{
		Triggered = false;

		if( !Sound )
			return;

		if( Sound->Playing() )
		{
			Sound->Stop();
		}
	}

	virtual void Context() override
	{
		ImGui::Text( "Audio" );
		ImGui::Text( "%s", Name.length() > 0 && Sound != nullptr ? Name.c_str() : "no sound selected" );
		
		ImGui::Separator();
		int InputLength = (int) Length;
		if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
		{
			Length = static_cast<Timecode>( InputLength );
		}

		if( ImGui::InputFloat( "Volume", &Volume, 1.0f, 10.0f, "%.0f" ) )
		{
			if( Sound && Sound->Playing() )
			{
				Sound->Volume( Volume );
			}
		}

		if( ImGui::InputFloat( "Rate", &Rate, 0.1f, 0.2f, "%.1f" ) )
		{
			if( Sound && Sound->Playing() )
			{
				Sound->Rate( Rate );
			}
		}
		
		ImGui::InputFloat( "Fade-In", &FadeIn, 1.0f, 10.0f, "%.0f" );
		ImGui::InputFloat( "Fade-Out", &FadeOut, 1.0f, 10.0f, "%.0f" );

		ImGui::Separator();

		if( ImGui::BeginCombo( "##SoundAssets", Name.c_str() ) )
		{
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

			ImGui::EndCombo();
		}
	}

	virtual const char* GetName() override
	{
		return Name.c_str();
	}

	virtual const char* GetType() override
	{
		return "Audio";
	}

	bool Triggered = false;
	std::string Name = std::string();
	CSound* Sound = nullptr;

	float FadeIn = 0.0f;
	float FadeOut = 0.1f;
	float Volume = 100.0f;
	float Rate = 1.0f;

	virtual void Export( CData& Data )
	{
		FTrackEvent::Export( Data );

		DataString::Encode( Data, Name );
		Data << Triggered;
		Data << FadeIn;
		Data << FadeOut;
		Data << Volume;
		Data << Rate;

		if( Sound->FileLocation.length() > 0 )
		{
			if( Sound->GetSoundType() == ESoundType::Memory )
			{
				DataMarker::Mark( Data, "Sound" );
			}
			else
			{
				DataMarker::Mark( Data, "Stream" );
			}
			
			DataString::Encode( Data, Sound->FileLocation );
		}
	}

	virtual void Import( CData& Data )
	{
		FTrackEvent::Import( Data );

		DataString::Decode( Data, Name );
		Data >> Triggered;
		Data >> FadeIn;
		Data >> FadeOut;
		Data >> Volume;
		Data >> Rate;

		if( DataMarker::Check( Data, "Sound" ) )
		{
			std::string FileLocation;
			DataString::Decode( Data, FileLocation );
			Sound = CAssets::Get().CreateNamedSound( Name.c_str(), FileLocation.c_str() );
		}
		else if( DataMarker::Check( Data, "Stream" ) )
		{
			std::string FileLocation;
			DataString::Decode( Data, FileLocation );
			Sound = CAssets::Get().CreateNamedStream( Name.c_str(), FileLocation.c_str() );
		}
		
		Sound = CAssets::Get().FindSound( Name );
	}
};

struct FEventCamera : FTrackEvent
{
	void BlendCameras()
	{
		BlendResult = Camera;
		
		if( !Blend )
		{
			return;
		}

		auto& BlendSetup = BlendResult.GetCameraSetup();
		const auto& CameraSetupA = Camera.GetCameraSetup();
		const auto& CameraSetupB = CameraB.GetCameraSetup();
		const float Factor = MarkerToTime( Offset ) / MarkerToTime( Length );
		
		BlendSetup.CameraPosition = Math::Lerp( CameraSetupA.CameraPosition, CameraSetupB.CameraPosition, Factor );
		BlendSetup.FieldOfView = Math::Lerp( CameraSetupA.FieldOfView, CameraSetupB.FieldOfView, Factor );
		BlendResult.SetCameraOrientation( Math::Lerp( Camera.CameraOrientation, CameraB.CameraOrientation, Factor ) );
		BlendSetup.CameraDirection = Math::Lerp( CameraSetupA.CameraDirection, CameraSetupB.CameraDirection, Factor );
		
		BlendResult.Update();
	}
	
	virtual void Execute() override
	{
		auto* World = CWorld::GetPrimaryWorld();
		if( !World )
			return;

		BlendCameras();
		ActiveTimelineCamera = &BlendResult;

		if( TimelineCamera && World->GetActiveCamera() != &BlendResult )
		{
			World->SetActiveCamera( &BlendResult, 200000 );
		}
	}

	virtual void Reset() override
	{
		auto* World = CWorld::GetPrimaryWorld();
		if( !World )
			return;

		// if( World->GetActiveCamera() == &Camera )
		{
			World->SetActiveCamera( nullptr );
		}
	}

	static void ActiveCameraToTarget( CCamera& Target )
	{
		const auto* World = CWorld::GetPrimaryWorld();
		if( World )
		{
			const auto* ActiveCamera = World->GetActiveCamera();
			if( ActiveCamera != nullptr && ActiveCamera != &Target )
			{
				Target = *ActiveCamera;
			}
		}
	}

	virtual void Context() override
	{
		ImGui::Text( "Camera" );

		const auto& CameraSetup = Camera.GetCameraSetup();
		ImGui::Text( "Location: %.2f %.2f %.2f", CameraSetup.CameraPosition.X, CameraSetup.CameraPosition.Y, CameraSetup.CameraPosition.Z );
		ImGui::Text( "FOV: %.2f", CameraSetup.FieldOfView );

		ImGui::Separator();
		int InputLength = ( int) Length;
		if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
		{
			Length = static_cast<Timecode>( InputLength );
		}

		if( ImGui::Button( "View" ) )
		{
			auto* World = CWorld::GetPrimaryWorld();
			if( World )
			{
				if( World->GetActiveCamera() != &Camera )
				{
					World->SetActiveCamera( &Camera, 1000 );
				}
			}
		}

		if( ImGui::Checkbox( "Enable Linear Blending", &Blend ) )
		{
			CameraB = Camera;
		}

		if( ImGui::Button( "Update Camera" ) )
		{
			ActiveCameraToTarget( Camera );
		}

		ImGui::Separator();

		if( Blend )
		{
			if( ImGui::Button( "Update Camera B" ) )
			{
				ActiveCameraToTarget( CameraB );
			}
		}

		ImGui::Separator();
		ImGui::Separator();

		if( ImGui::Button( "Apply to Active" ) )
		{
			const auto* World = CWorld::GetPrimaryWorld();
			if( World )
			{
				auto* ActiveCamera = World->GetActiveCamera();
				if( ActiveCamera && ActiveCamera != &BlendResult )
				{
					*ActiveCamera = BlendResult;
					auto ShuffleOrientation = Math::ToDegrees( Math::DirectionToEuler( BlendResult.GetCameraSetup().CameraDirection ) );
					ActiveCamera->CameraOrientation.X = ShuffleOrientation.Pitch;
					ActiveCamera->CameraOrientation.Y = ShuffleOrientation.Yaw;
					ActiveCamera->CameraOrientation.Z = ShuffleOrientation.Roll;
				}
			}
		}

		ImGui::Separator();
	}

	virtual const char* GetName() override
	{
		return "Camera";
	}

	virtual const char* GetType() override
	{
		return "Camera";
	}

	std::string Name = std::string();
	CCamera Camera;
	CCamera CameraB;
	CCamera BlendResult;
	bool Blend = false;

	virtual void Export( CData& Data )
	{
		FTrackEvent::Export( Data );

		DataString::Encode( Data, Name );
		Data << Camera;
		Data << CameraB;
		Data << Blend;
	}

	virtual void Import( CData& Data )
	{
		FTrackEvent::Import( Data );

		DataString::Decode( Data, Name );
		Data >> Camera;
		Data >> CameraB;
		Data >> Blend;
	}
};

struct FEventRenderable : FTrackEvent
{
	virtual void Execute() override
	{
		CWindow::Get().GetRenderer().QueueRenderable( &Renderable );
	}

	virtual void Reset() override
	{
		
	}

	virtual void Context() override
	{
		ImGui::Text( "Renderable" );

		ImGui::Separator();
		int InputLength = ( int) Length;
		if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
		{
			Length = static_cast<Timecode>( InputLength );
		}

		ImGui::Separator();

		if( ImGui::BeginCombo( "##MeshAssets", Name.c_str() ) )
		{
			auto& Assets = CAssets::Get();
			auto& Meshes = Assets.GetMeshes();
			for( auto& Pair : Meshes )
			{
				if( ImGui::Selectable( Pair.first.c_str() ) )
				{
					Name = Pair.first;
					Renderable.SetMesh( Pair.second );
				}

				ImGui::Separator();
			}

			ImGui::EndCombo();
		}

		if( ImGui::BeginCombo( "##ShaderAssets", Name.c_str() ) )
		{
			auto& Assets = CAssets::Get();
			auto& Shaders = Assets.GetShaders();
			for( auto& Pair : Shaders )
			{
				if( ImGui::Selectable( Pair.first.c_str() ) )
				{
					Renderable.SetShader( Pair.second );
				}

				ImGui::Separator();
			}

			ImGui::EndCombo();
		}

		if( ImGui::BeginCombo( "##TextureAssets", Name.c_str() ) )
		{
			auto& Assets = CAssets::Get();
			auto& Textures = Assets.GetTextures();
			for( auto& Pair : Textures )
			{
				if( ImGui::Selectable( Pair.first.c_str() ) )
				{
					Renderable.SetTexture( Pair.second, ETextureSlot::Slot0 );
				}

				ImGui::Separator();
			}

			ImGui::EndCombo();
		}

		auto& RenderData = Renderable.GetRenderData();
		auto& Transform = RenderData.Transform;
		auto Position = Transform.GetPosition();
		if( ImGui::InputFloat3( "##mp", &Position.X, "%.2f" ) ) //
		{
			Transform.SetPosition( Position );
		}

		auto Orientation = Transform.GetOrientation();
		if( ImGui::InputFloat3( "##mo", &Orientation.X, "%.2f" ) )
		{
			Transform.SetOrientation( Orientation );
		}
	}

	virtual const char* GetName() override
	{
		return Name.c_str();
	}

	virtual const char* GetType() override
	{
		return "Mesh";
	}

	std::string Name = std::string();
	CRenderable Renderable;

	virtual void Export( CData& Data )
	{
		FTrackEvent::Export( Data );

		DataString::Encode( Data, Name );
	}

	virtual void Import( CData& Data )
	{
		FTrackEvent::Import( Data );

		DataString::Decode( Data, Name );
	}
};

static std::map<std::string, std::function<FTrackEvent*()>> EventTypes
{
	std::make_pair( "Audio", CreateTrack<FEventAudio> ),
	std::make_pair( "Camera", CreateTrack<FEventCamera> ),
	std::make_pair( "Mesh", CreateTrack<FEventRenderable> ),
	std::make_pair( "Image", CreateTrack<FEventImage> )
};

void FTrackEvent::Evaluate( const Timecode& Marker )
{
	if( Marker >= Start && Marker < ( Start + Length ) )
	{
		Execute();
	}
}

void FTrackEvent::Context()
{
	ImGui::Text( "%s", GetName() );

	ImGui::Separator();
	int InputLength = StaticCast<int>( Length );
	if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
	{
		Length = static_cast<Timecode>( InputLength );
	}
}

void FTrackEvent::AddType( const std::string& Name, const std::function<FTrackEvent*()>& Generator )
{
	if( EventTypes.find( Name ) == EventTypes.end() )
	{
		EventTypes.insert_or_assign( Name, Generator );
	}
}

void FTrackEvent::UpdateInternalMarkers( const Timecode& Marker )
{
	if( Scrubbing )
	{
		PreviousOffset = 0;
	}
	else
	{
		PreviousOffset = Offset;
	}

	Offset = Math::Clamp( Marker - Start, StaticCast<Timecode>( 0 ), Length );
}

void FTrackEvent::Export( CData& Data )
{
	DataString::Encode( Data, GetType() );
	Data << Start;
	Data << Length;
}

void FTrackEvent::Import( CData& Data )
{
	std::string Type;
	DataString::Decode( Data, Type );
	*this = *EventTypes[Type]();
	Data >> Start;
	Data >> Length;
}

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
				Event->UpdateInternalMarkers( Marker );
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

	friend CData& operator<<( CData& Data, FTrack& Track )
	{
		DataString::Encode( Data, "Track" );
		Data << Track.Start;
		Data << Track.Length;

		// DataVector::Encode( Data, Track.Events );
		DataString::Encode( Data, "Events" );
		uint32_t Events = Track.Events.size();
		Data << Events;

		for( auto& Event : Track.Events )
		{
			DataString::Encode( Data, Event->GetType() );
			Data << Event;
		}

		return Data;
	}

	friend CData& operator>>( CData& Data, FTrack& Track )
	{
		std::string String;
		DataString::Decode( Data, String );
		Data >> Track.Start;
		Data >> Track.Length;

		DataString::Decode( Data, String );
		uint32_t Events = 0;
		Data >> Events;

		for( uint32_t Index = 0; Index < Events; Index++ )
		{
			DataString::Decode( Data, String );
			
			FTrackEvent* Event = EventTypes[String]();
			Track.Events.emplace_back( Event );

			Data >> Event;
		}

		return Data;
	}
};

class CTimeline
{
public:
	CTimeline( Timecode Start = 0, Timecode End = 0 )
	{
		StartMarker = Start;
		EndMarker = End;
		Marker = 0;
		Status = ESequenceStatus::Stopped;

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
			File.Load( true );
			File.Extract( *this );

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
			Marker = StartMarker;

			for( auto& Track : Tracks )
			{
				Track.Reset();
			}
		}

		Status = ESequenceStatus::Playing;
	}

	void Pause()
	{
		if( Status == ESequenceStatus::Paused || Status == ESequenceStatus::Stopped )
		{
			Status = ESequenceStatus::Playing;

			Scrubbing = true;
			for( auto& Track : Tracks )
			{
				Track.Reset();
				Track.Evaluate( Marker );
			}
			Scrubbing = false;

			StartTime = GameLayersInstance->GetRealTime() - MarkerToTime( Marker - StartMarker );
			
			return;
		}
		
		Status = ESequenceStatus::Paused;

		for( auto& Track : Tracks )
		{
			Track.Reset();
		}
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
		return Status == ESequenceStatus::Stopped || Status == ESequenceStatus::Paused;
	}

	void Step()
	{
		if( Status < ESequenceStatus::Playing )
			return;

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
		StartTime = GameLayersInstance->GetRealTime() - MarkerToTime( Marker - StartMarker );

		const bool CachedScrubbingState = Scrubbing;
		Scrubbing = true;		
		for( auto& Track : Tracks )
		{
			Track.Reset();
			Track.Evaluate( Marker );
		}
		Scrubbing = CachedScrubbingState;
	}

	Timecode CurrentMarker() const
	{
		return Marker;
	}

	Timecode Size() const
	{
		return StartMarker - EndMarker;
	}

	double Time() const
	{
		return MarkerToTime( Marker );
	}

	double Length() const
	{
		return MarkerRangeToTime( StartMarker, EndMarker );
	}

	double PreviousTime = 0.0f;
	double Remainder = 0.0f;
	
	void Feed()
	{
		const auto RealTime = GameLayersInstance->GetRealTime();
		const auto DebugTime = RealTime - StartTime;
		const auto Drift = Math::Max( 0.0, DebugTime - Time() );
		const auto StepDrift = StaticCast<uint64_t>( std::ceil( Drift * StaticCast<double>( Timebase ) ) );
		const auto DeltaTime = RealTime - PreviousTime + Remainder;
		auto Steps = StaticCast<uint64_t>( std::floor( DeltaTime * StaticCast<double>( Timebase ) ) );
		Steps += StepDrift;
		
		for( uint64_t StepIndex = 0; StepIndex < Steps; StepIndex++ )
		{
			if( Status == ESequenceStatus::Stopped )
				break;

			Step();
		}

		
		/*const std::string DebugString =
			"Time Since Play        : " + std::to_string( DebugTime ) +
			"\nReconstructed Time: " + std::to_string( Time() ) +
			"\nDrift                  : " + std::to_string( Drift ) +
			"\nStep Drift             : " + std::to_string( StepDrift ) +
			"\nSteps                  : " + std::to_string( Steps )
			;

		UI::AddText( Vector2D( 51.0f, 201.0f ), DebugString.c_str(), nullptr, Color::Black );
		UI::AddText( Vector2D( 50.0f, 200.0f ), DebugString.c_str() );*/
		
		PreviousTime = RealTime;
	}

	void Frame()
	{
		// Clear the active timeline camera.
		ActiveTimelineCamera = nullptr;
		
		// Run all the timeline events that are associated with the current marker.
		if( Status != ESequenceStatus::Stopped )
		{
			for( auto& Track : Tracks )
			{
				Track.Evaluate( Marker );
			}
		}
		
		Scrubbing = false;

		// Always evaluate cameras if the timeline isn't drawn.
		if( !DrawTimeline )
		{
			TimelineCamera = true;
		}

		// Make sure we clear the active camera.
		if( !ActiveTimelineCamera && Playing() )
		{
			auto World = CWorld::GetPrimaryWorld();
			if( World )
			{
				World->SetActiveCamera( nullptr );
			}
		}

		if( DrawTimeline )
		{
			DisplayTimeline();
		}

		Feed();
	}

	void Draw()
	{
		DrawTimeline = true;
	}

	void DisplayTimeline()
	{
		if( ImGui::Begin(
			"Timeline",
			&DrawTimeline,
			ImVec2( 1000.0f, 500.0f ), 0.45f
		) )
		{
			static FTrack* ActiveTrack = nullptr;
			static std::vector<FTrackEvent*> ActiveEvents;
			static int SequenceLengthSeconds = 2;

			/*const bool ShouldPlayPause = ImGui::IsKeyReleased( ImGui::GetKeyIndex( ImGuiKey_Space ) );
			if( ShouldPlayPause )
			{
				if( Status == ESequenceStatus::Stopped )
				{
					Play();
				}
				else
				{
					Pause();
				}
			}*/

			if( ImGui::Button( "Play" ) )
			{
				Play();

				StartTime = GameLayersInstance->GetRealTime();
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

			if( ImGui::Button( "Step" ) )
			{
				// Increment the timeline step.
				Marker++;
			}

			ImGui::SameLine();

			if( ImGui::Button( "Save" ) )
			{
				CData Data;
				Data << *this;

				CFile File( Location.c_str() );
				File.Load( Data );
				File.Save();
			}

			if( ImGui::Button( "Create Track" ) )
			{
				CreateTrack();
				ActiveTrack = nullptr;
			}

			ImGui::SameLine();

			static std::string EventType = ( *EventTypes.find( "Audio" ) ).first;

			if( ImGui::Button( "Create Event" ) )
			{
				if( ActiveTrack && Tracks.size() > 0 )
				{
					FTrackEvent* Event = EventTypes[EventType]();
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

			ImGui::SameLine();

			if( ImGui::BeginCombo( "##EventTypes", EventType.c_str() ) )
			{
				for( auto& Pair : EventTypes )
				{
					if( ImGui::Selectable( Pair.first.c_str() ) )
					{
						EventType = Pair.first;
					}

					ImGui::Separator();
				}

				ImGui::EndCombo();
			}

			static float StretchFactor = 1.0f;
			if( ImGui::Button( "Stretch Sequence" ) )
			{
				Stretch( StretchFactor );				
			}

			ImGui::SameLine();

			ImGui::InputFloat( "##StretchFloat", &StretchFactor, 0.1f, 1.0f, "%.1f" );

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

			static float WidthScale = 1.0f;
			size_t WidthDelta = Math::Max( 1.0f, static_cast<size_t>( 1000.0f / 30 ) * WidthScale );
			ImGui::SliderFloat( "Scale", &WidthScale, 0.1f, 10.0f );

			if( ImGui::GetIO().KeyCtrl )
			{
				WidthScale = Math::Max( 0.1f, WidthScale + ImGui::GetIO().MouseWheel * 0.33f );
			}

			ImGui::SameLine();
			if( ImGui::Checkbox( "Use Timeline Cameras", &TimelineCamera ) )
			{
				if( !TimelineCamera )
				{
					auto World = CWorld::GetPrimaryWorld();
					if( World )
					{
						World->SetActiveCamera( nullptr );
					}
				}
			}

			auto MarkerLocalPosition = ( static_cast<float>( Marker ) / static_cast<float>( Timebase ) ) * WidthDelta;
			auto EndMarkerLocalPosition = ( static_cast<float>( EndMarker ) / static_cast<float>( Timebase ) ) * WidthDelta;

			auto CursorOffset = 100.0f;

			auto WindowWidth = ImGui::GetWindowWidth() - CursorOffset;
			auto MarkerWindowPosition = MarkerLocalPosition;
			auto MarkerPercentageWindow = MarkerWindowPosition / WindowWidth;
			auto EndMarkerPercentageWindow = EndMarkerLocalPosition / WindowWidth;

			if( Status != ESequenceStatus::Stopped && EndMarkerPercentageWindow > 1.0f && MarkerPercentageWindow > 0.5f )
			{
				CursorOffset -= MarkerWindowPosition - WindowWidth * 0.5f;
			}

			UI::AddText( Vector2D( 50.0f, 130.0f ), "MarkerWindowPosition", MarkerWindowPosition );
			UI::AddText( Vector2D( 50.0f, 150.0f ), "MarkerPercentageWindow", MarkerPercentageWindow );

			auto CursorPosition = ImGui::GetCursorPos();
			CursorPosition.x += CursorOffset;

			size_t Bars = ( ( EndMarker - StartMarker ) / Timebase );
			for( size_t BarIndex = 0; BarIndex < Bars; )
			{
				auto BarPosition = CursorPosition;
				BarPosition.x += BarIndex * WidthDelta;
				ImGui::SetCursorPos( BarPosition );

				ImGui::Text( "%zi", BarIndex );

				if( WidthScale > 0.5f )
				{
					BarIndex++;
				}
				else
				{
					BarIndex += 5;
				}
			}

			auto BarPosition = ImGui::GetCursorPos();
			ImGui::SetCursorPos( CursorPosition );
			auto MouseStartPosition = ImGui::GetCursorScreenPos();

			if( Bars > 0 && ImGui::InvisibleButton( "MarkerTracker", ImVec2( Bars * WidthDelta, 30.0f ) ) )
			{
				auto MousePosition = ImGui::GetMousePos();
				auto MouseOffset = MousePosition.x - MouseStartPosition.x;
				if( MouseOffset < 0.0f )
					MouseOffset = 0.0f;

				GoTo( static_cast<Timecode>( ( MouseOffset / WidthDelta ) * Timebase ) );
			}

			Timecode GhostMarker = 0;
			if( ImGui::IsItemHovered() )
			{
				auto MousePosition = ImGui::GetMousePos();
				auto MouseOffset = MousePosition.x - MouseStartPosition.x;
				if( MouseOffset < 0.0f )
					MouseOffset = 0.0f;

				GhostMarker = static_cast<Timecode>( ( MouseOffset / WidthDelta ) * Timebase );
				if( ImGui::GetIO().MouseDown[0] )
				{
					GoTo( GhostMarker );
					Scrubbing = true;

					if( Status == ESequenceStatus::Stopped )
					{
						Status = ESequenceStatus::Paused;
					}
				}
			}

			ImGui::SetCursorPos( BarPosition );

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

					bool SelectedEvent = false;
					for( auto& ActiveEvent : ActiveEvents )
					{
						if( ActiveEvent == Event )
						{
							SelectedEvent = true;
						}
					}

					const float Brightness = SelectedEvent ? 1.0f : 0.6f;
					ImGui::PushStyleColor( ImGuiCol_Button, static_cast<ImVec4>( ImColor::HSV( Hue, 0.6f, Brightness ) ) );
					ImGui::PushStyleColor( ImGuiCol_ButtonHovered, static_cast<ImVec4>( ImColor::HSV( Hue, 0.6f, 0.9f ) ) );
					ImGui::PushStyleColor( ImGuiCol_ButtonActive, static_cast<ImVec4>( ImColor::HSV( Hue, 0.4f, 0.9f ) ) );
					ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 0.0f );
					ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );

					std::string EventName = std::string( Event->GetName() ) +
						" (Length: " +
						std::to_string( Event->Length / Timebase ) +
						")"
						;

					std::string EventLabel = EventName + "##eb" + std::to_string( GlobalIndex );
					ImGui::Button( EventLabel.c_str(), ImVec2( ( static_cast<float>( Event->Length ) / static_cast<float>( Timebase ) ) * WidthDelta, 30.0f ) );
					if( ImGui::IsItemHovered() )
					{
						ImGui::SetTooltip( EventName.c_str() );
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
						if( !ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift )
						{
							ActiveEvents.clear();
						}

						ActiveTrack = &Track;
						ActiveEvents.emplace_back( Event );
					}

					ImVec2 Drag = ImVec2();

					if( ImGui::IsItemHovered() && ImGui::GetIO().MouseDown[0] )
					{
						auto Delta = ImGui::GetMouseDragDelta( 0, 0.0f );
						Drag.x += Delta.x;
						Drag.y += Delta.y;
						ImGui::ResetMouseDragDelta( 0 );

						// Marker = Event->Start;
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
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();

					EventIndex++;
					GlobalIndex++;
				}

				TrackIndex++;

				ImGui::EndChild();

				ImGui::PopStyleColor();
			}

			auto* DrawList = ImGui::GetWindowDrawList();
			if( DrawList )
			{
				auto VerticalOffset = 32.0f;
				ScreenPosition.x += CursorOffset;

				auto MarkerPosition = ScreenPosition;
				MarkerPosition.x += MarkerLocalPosition;
				auto LineEnd = MarkerPosition;
				LineEnd.y += Tracks.size() * VerticalOffset;
				DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32_WHITE );

				UI::AddText( Vector2D( 50.0f, 50.0f ), "ScreenPositionX", ScreenPosition.x );
				UI::AddText( Vector2D( 50.0f, 70.0f ), "MarkerPosition", MarkerPosition.x );
				UI::AddText( Vector2D( 50.0f, 90.0f ), "WindowWidth", WindowWidth );
				UI::AddText( Vector2D( 50.0f, 110.0f ), "CursorPosition", CursorPosition.x );

				MarkerPosition = ScreenPosition;
				MarkerPosition.x += ( static_cast<float>( GhostMarker ) / static_cast<float>( Timebase ) ) * WidthDelta;
				LineEnd = MarkerPosition;
				LineEnd.y += Tracks.size() * VerticalOffset;
				DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32( 128, 128, 128, 255 ) );

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

			if( !ActiveEvents.empty() )
			{
				// ImGui::GetIO().WantCaptureKeyboard = true;
			}

			if( ImGui::IsKeyReleased( ImGui::GetKeyIndex( ImGuiKey_Delete ) ) )
			{
				for( auto& ActiveEvent : ActiveEvents )
				{
					if( ActiveEvent )
					{
						for( auto& Track : Tracks )
						{
							auto Iterator = std::find( Track.Events.begin(), Track.Events.end(), ActiveEvent );
							if( Iterator != Track.Events.end() )
							{
								Track.Events.erase( Iterator );
								ActiveEvent = nullptr;
							}
						}
					}
				}

				ActiveEvents.clear();
			}

			// Update the length of each track.
			for( auto& Track : Tracks )
			{
				Track.Length = EndMarker - StartMarker;

				for( auto* Event : Track.Events )
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

		ImGui::End();
	}

	void CreateTrack()
	{
		FTrack Track;
		Track.Start = StartMarker;
		Track.Length = EndMarker - StartMarker;

		Tracks.emplace_back( Track );
	}

	void Stretch( const float& Factor )
	{
		EndMarker *= Factor;
		Marker *= Factor;

		for( auto& Track : Tracks )
		{
			Track.Length *= Factor;
			Track.Start *= Factor;
			
			for( auto& Event : Track.Events )
			{
				Event->Length *= Factor;
				Event->Start *= Factor;
			}
		}
	}

	friend CData& operator<<( CData& Data, CTimeline& Timeline )
	{
		DataString::Encode( Data, Timeline.Location );

		Data << Timeline.StartMarker;
		Data << Timeline.EndMarker;

		DataVector::Encode( Data, Timeline.Tracks );

		return Data;
	}

	friend CData& operator>>( CData& Data, CTimeline& Timeline )
	{
		DataString::Decode( Data, Timeline.Location );

		Data >> Timeline.StartMarker;
		Data >> Timeline.EndMarker;

		DataVector::Decode( Data, Timeline.Tracks );

		return Data;
	}

protected:
	bool DrawTimeline;
	double StartTime = -1.0f;

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
		Timeline->Load( FileLocation );
	}

	return true;
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

CData& operator<<( CData& Data, CSequence& Sequence )
{
	Data << *Sequence.Timeline;
	return Data;
}

CData& operator>>( CData& Data, CSequence& Sequence )
{
	Data >> *Sequence.Timeline;
	return Data;
}
