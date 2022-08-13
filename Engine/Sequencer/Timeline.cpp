// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Timeline.h"

#include <Engine/Application/AssetHelper.h>
#include <Engine/Application/ApplicationMenu.h>

#include <Engine/Animation/Animator.h>
#include <Engine/Audio/Sound.h>

#include <Engine/Display/UserInterface.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Input/InputMapGLFW.h>

#include <Engine/Sequencer/Events/ImageEvent.h>
#include <Engine/Sequencer/Events/EntityEvent.h>

#include <Engine/Utility/Gizmo.h>
#include <Engine/Utility/Locator/InputLocator.h>

#include <Engine/World/World.h>
#include <Engine/World/Entity/LightEntity/LightEntity.h>

// static bool TimelineCamera = true;
// static CCamera* ActiveTimelineCamera = nullptr;
constexpr uint32_t SequenceCameraPriority = 200000;

double MarkerToTime( const Timecode& Marker )
{
	return StaticCast<double>( Marker ) / StaticCast<double>( Timebase );
}

double MarkerRangeToTime( const Timecode& StartMarker, const Timecode& EndMarker )
{
	return StaticCast<double>( EndMarker - StartMarker ) / StaticCast<double>( Timebase );
}

struct FEventAudio : TrackEvent
{
	void Evaluate( const Timecode& Marker ) override
	{
		if( !Sound )
		{
			Sound = CAssets::Get().Sounds.Find( Name );
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
				if( Timeline->Scrubbing )
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

	void Execute() override
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
			Information.Bus = Bus;
			Sound->Start( Information );
		}

		if( Timeline->Scrubbing )
		{
			Sound->Offset( MarkerToTime( Offset ) );
		}
	}

	void Reset() override
	{
		Triggered = false;

		if( !Sound )
			return;

		if( Sound->Playing() )
		{
			Sound->Stop();
		}
	}

	void Context() override
	{
		ImGui::Text( "Audio" );
		ImGui::Text( "%s", Name.length() > 0 && Sound != nullptr ? Name.c_str() : "no sound selected" );

		ImGui::Separator();
		int InputLength = StaticCast<int>( Length );
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

		Bus = ImGui::BusSelector( Bus );

		ImGui::Separator();

		static AssetDropdownData Data;
		Data.Name = Name;
		DisplayAssetDropdown( EAsset::Sound, Data );
		Data.Assign( Name, Sound );
	}

	const char* GetName() override
	{
		return Name.c_str();
	}

	const char* GetType() const override
	{
		return "Audio";
	}

	bool Triggered = false;
	std::string Name = std::string();
	CSound* Sound = nullptr;
	Bus::Type Bus = Bus::UI;

	float FadeIn = 0.0f;
	float FadeOut = 0.1f;
	float Volume = 100.0f;
	float Rate = 1.0f;

	void Export( CData& Data ) const override
	{
		TrackEvent::Export( Data );

		DataString::Encode( Data, Name );
		Data << Triggered;
		Data << FadeIn;
		Data << FadeOut;
		Data << Volume;
		Data << Rate;

		if( Sound && Sound->FileLocation.length() > 0 )
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

		DataMarker::Mark( Data, "Bus" );
		Data << Bus;

	}

	void Import( CData& Data ) override
	{
		TrackEvent::Import( Data );

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

		Sound = CAssets::Get().Sounds.Find( Name );

		if( DataMarker::Check( Data, "Bus" ) )
		{
			Data >> Bus;
		}
	}
};

CCamera CopiedCamera;
struct FEventCamera : TrackEvent
{
	void BlendCameras()
	{
		BlendResult = Camera;

		// Check if this event wants to use camera blending.
		if( !Blend )
			return;

		if( Bezier )
		{
			const auto Factor = static_cast<float>( MarkerToTime( Offset ) / MarkerToTime( Length ) );
			BlendResult = CCamera::BezierBlend( Camera, Tangent, CameraB, TangentB, Factor );
		}
		else
		{
			const auto Factor = static_cast<float>( MarkerToTime( Offset ) / MarkerToTime( Length ) );
			BlendResult = CCamera::Lerp( Camera, CameraB, Factor );
		}
	}

	void Execute() override
	{
		auto* World = CWorld::GetPrimaryWorld();
		if( !World )
			return;

		static auto PreviousResult = BlendResult;
		BlendCameras();

		if( HandheldFactor > 0.0f )
		{
			const auto HandheldShift = 1.0 - Math::Abs( MarkerToTime( Offset ) / MarkerToTime( Length ) ) * 2.0 - 1.0;
			const auto HandheldTime = MarkerToTime( StoredMarker );
			BlendResult = CCamera::HandheldSimulation( BlendResult, HandheldFactor, HandheldTime * HandheldSpeed );
		}

		PreviousResult = BlendResult;

		Timeline->ActiveCamera = &BlendResult;

		if( Timeline->EnableCamera && World->GetActiveCamera() != &BlendResult )
		{
			World->SetActiveCamera( &BlendResult, SequenceCameraPriority );
		}
	}

	void Reset() override
	{
		auto* World = CWorld::GetPrimaryWorld();
		if( !World )
			return;

		if( Timeline->EnableCamera && !Timeline->Scrubbing && World->GetActiveCamera() == &BlendResult )
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

	void Context() override
	{
		ImGui::Text( "Camera" );

		const auto& CameraSetup = Camera.GetCameraSetup();
		ImGui::Text( "Location: %.2f %.2f %.2f", CameraSetup.CameraPosition.X, CameraSetup.CameraPosition.Y, CameraSetup.CameraPosition.Z );
		ImGui::Text( "FOV: %.2f", CameraSetup.FieldOfView );

		ImGui::Separator();
		int InputLength = StaticCast<int>( Length );
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
					World->SetActiveCamera( &Camera, SequenceCameraPriority );
				}
			}
		}

		if( ImGui::Checkbox( "Enable Linear Blending", &Blend ) )
		{
			CameraB = Camera;
		}

		if( Blend )
		{
			ImGui::Checkbox( "Enable Bezier Blending", &Bezier );
		}

		if( ImGui::Button( "Update Camera" ) )
		{
			ActiveCameraToTarget( Camera );
		}

		ImGui::SameLine();
		if( ImGui::Button( "Copy##CopyA" ) )
		{
			CopiedCamera = Camera;
		}

		ImGui::SameLine();
		if( ImGui::Button( "Paste##PasteA" ) )
		{
			Camera = CopiedCamera;
		}

		if( Bezier )
		{
			ImGui::SameLine();
			ImGui::DragFloat( "Tangent", &Tangent, 0.1f );
		}

		bool ShouldUpdateA = false;
		if( ImGui::DragFloat3( "Position##CamPosA", &Camera.GetCameraSetup().CameraPosition[0], 0.1f ) )
		{
			ShouldUpdateA = true;
		}

		if( ImGui::DragFloat3( "Orientation##CamRotA", &Camera.CameraOrientation[0] ) )
		{
			ShouldUpdateA = true;
			Camera.SetCameraOrientation( Camera.CameraOrientation );
		}

		if( ImGui::DragFloat( "Field of View##CamFOVA", &Camera.GetCameraSetup().FieldOfView ) )
		{
			ShouldUpdateA = true;
		}

		if( ShouldUpdateA )
		{
			Camera.Update();
		}

		ImGui::Separator();

		if( Blend )
		{
			if( ImGui::Button( "Update Camera B" ) )
			{
				ActiveCameraToTarget( CameraB );
			}

			ImGui::SameLine();
			if( ImGui::Button( "Copy##CopyB" ) )
			{
				CopiedCamera = CameraB;
			}

			ImGui::SameLine();
			if( ImGui::Button( "Paste##PasteB" ) )
			{
				CameraB = CopiedCamera;
			}

			if( Bezier )
			{
				ImGui::SameLine();
				ImGui::DragFloat( "Tangent##TangentB", &TangentB, 0.1f );
			}

			bool ShouldUpdateB = false;
			if( ImGui::DragFloat3( "Position##CamPosB", &CameraB.GetCameraSetup().CameraPosition[0], 0.1f ) )
			{
				ShouldUpdateB = true;
			}

			if( ImGui::DragFloat3( "Orientation##CamRotB", &CameraB.CameraOrientation[0] ) )
			{
				ShouldUpdateB = true;
				CameraB.SetCameraOrientation( CameraB.CameraOrientation );
			}

			if( ImGui::DragFloat( "Field of View##CamFOVB", &CameraB.GetCameraSetup().FieldOfView ) )
			{
				ShouldUpdateB = true;
			}

			if( ShouldUpdateB )
			{
				CameraB.Update();
			}

			if( ImGui::Button( "Swap Cameras" ) )
			{
				const auto CameraTemp = Camera;
				Camera = CameraB;
				CameraB = CameraTemp;
			}

			if( ImGui::IsItemHovered() )
			{
				ImGui::BeginTooltip();
				ImGui::Text( "Swaps the two cameras." );
				ImGui::EndTooltip();
			}
		}

		ImGui::Separator();

		ImGui::DragFloat( "Handheld Simulation", &HandheldFactor, 0.01f, 0.0f, 10.0f );
		ImGui::DragFloat( "Handheld Speed", &HandheldSpeed, 0.01f, 0.0f, 10.0f );

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

					const auto ShuffleOrientation = Math::ToDegrees( Math::DirectionToEuler( BlendResult.GetCameraSetup().CameraDirection ) );
					ActiveCamera->CameraOrientation.X = ShuffleOrientation.Pitch;
					ActiveCamera->CameraOrientation.Y = ShuffleOrientation.Yaw;
					ActiveCamera->CameraOrientation.Z = ShuffleOrientation.Roll;
				}
			}
		}

		ImGui::Separator();
	}

	void Visualize() override
	{
		if( Blend )
		{
			CCamera Visualization = Camera;
			CCamera VisualizationPrevious = Camera;
			constexpr Timecode Iterations = 20;
			const auto Delta = Length / Iterations;
			for( Timecode Index = 0; Index < Iterations; Index++ )
			{
				const auto Factor = static_cast<float>( MarkerToTime( Index * Delta ) / MarkerToTime( Length ) );

				if( Bezier )
				{
					Visualization = CCamera::BezierBlend( Camera, Tangent, CameraB, TangentB, Factor );
				}
				else
				{
					Visualization = CCamera::Lerp( Camera, CameraB, Factor );
				}

				const auto PositionA = VisualizationPrevious.GetCameraPosition();
				const auto PositionB = Visualization.GetCameraPosition();

				UI::AddLine( PositionA, PositionB, Color::Green );
				UI::AddLine( PositionB, PositionB + Visualization.GetCameraSetup().CameraDirection * 5.0f, Color::Red );
				UI::AddCircle( Visualization.GetCameraPosition(), 3.0f, Color::Yellow );

				VisualizationPrevious = Visualization;
			}

			PointGizmo( &CameraB.GetCameraSetup().CameraPosition, CameraDragGizmoB );
		}

		UI::AddLine( BlendResult.GetCameraPosition(), BlendResult.GetCameraPosition() + BlendResult.GetCameraSetup().CameraDirection * 5.0f, Color::Purple );
		UI::AddCircle( BlendResult.GetCameraPosition(), 3.0f, Color::Blue );
		BlendResult.DrawFrustum();

		PointGizmo( &Camera.GetCameraSetup().CameraPosition, CameraDragGizmoA );
	}

	const char* GetName() override
	{
		return "Camera";
	}

	const char* GetType() const override
	{
		return "Camera";
	}

	std::string Name = std::string();
	CCamera Camera;
	float Tangent = 1.0f;

	CCamera CameraB;
	float TangentB = 1.0f;

	CCamera BlendResult;
	bool Blend = false;
	bool Bezier = false;

	float HandheldFactor = 0.0f;
	float HandheldSpeed = 1.0f;

	void Export( CData& Data ) const override
	{
		TrackEvent::Export( Data );

		DataString::Encode( Data, Name );
		Data << Camera;
		Data << CameraB;
		Data << Blend;

		// Bezier blending.
		Serialize::Export( Data, "bz", Bezier );
		Serialize::Export( Data, "tn", Tangent );
		Serialize::Export( Data, "tn", TangentB );

		Serialize::Export( Data, "hd", HandheldFactor );
		Serialize::Export( Data, "hs", HandheldSpeed );
	}

	void Import( CData& Data ) override
	{
		TrackEvent::Import( Data );

		DataString::Decode( Data, Name );
		Data >> Camera;
		Data >> CameraB;
		Data >> Blend;

		// Bezier blending.
		Serialize::Import( Data, "bz", Bezier );
		Serialize::Import( Data, "tn", Tangent );
		Serialize::Import( Data, "tn", TangentB );

		Serialize::Import( Data, "hd", HandheldFactor );
		Serialize::Import( Data, "hs", HandheldSpeed );
	}

private:
	// For editing.
	DragVector CameraDragGizmoA;

	// For editing.
	DragVector CameraDragGizmoB;
};

struct FEventRenderable : TrackEvent
{
	void Execute() override
	{
		auto* Mesh = Renderable.GetMesh();
		if( !Mesh )
			return;

		// Make sure the mesh is up to date.
		Animation.Mesh = Mesh;

		const auto IsAnimating =
			Animation.Mesh &&
			!Animation.CurrentAnimation.empty() &&
			!Animation.Mesh->GetSkeleton().Animations.empty()
			;

		if( IsAnimating )
		{
			// Calculate the animation time relative to the event.
			const auto AnimationTime = MarkerToTime( Offset ) * PlayRate;

			// Apply the event-relative animation time.
			Animation.LoopAnimation = LoopAnimation;
			Animation.Time = static_cast<float>( AnimationTime );
			Animation.PlayRate = 0.0f;

			Animator::Update( Animation, 0.0, true );
			Animator::Submit( Animation, &Renderable );
		}

		auto& RenderData = Renderable.GetRenderData();
		if( IsAnimating )
		{
			RenderData.WorldBounds = Animation.CalculateBounds( RenderData.Transform );
		}
		else
		{
			RenderData.WorldBounds = Math::AABB( Mesh->GetBounds(), RenderData.Transform );
		}

		const auto LightOrigin = RenderData.WorldBounds.Center();
		RenderData.LightIndex = LightEntity::Fetch( LightOrigin );

		CWindow::Get().GetRenderer().QueueRenderable( &Renderable );
	}

	void Reset() override
	{

	}

	void Context() override
	{
		ImGui::Text( "Renderable" );

		ImGui::Separator();
		int InputLength = StaticCast<int>( Length );
		if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
		{
			Length = static_cast<Timecode>( InputLength );
		}

		ImGui::Separator();

		static AssetDropdownData MeshData;
		MeshData.Name = MeshName;
		DisplayAssetDropdown( EAsset::Mesh, MeshData );

		CMesh* AssignedMesh = nullptr;
		MeshData.Assign( MeshName, AssignedMesh );

		if( AssignedMesh )
		{
			Renderable.SetMesh( AssignedMesh );
			Animation.Mesh = AssignedMesh;
		}

		static AssetDropdownData ShaderData;
		ShaderData.Name = ShaderName;
		DisplayAssetDropdown( EAsset::Shader, ShaderData );

		CShader* AssignedShader = nullptr;
		ShaderData.Assign( ShaderName, AssignedShader );

		if( AssignedShader )
		{
			Renderable.SetShader( AssignedShader );
		}

		static AssetDropdownData TextureData;
		TextureData.Name = TextureName;
		DisplayAssetDropdown( EAsset::Texture, TextureData );

		CTexture* AssignedTexture = nullptr;
		TextureData.Assign( TextureName, AssignedTexture );

		if( AssignedTexture )
		{
			Renderable.SetTexture( AssignedTexture, ETextureSlot::Slot0 );
		}

		auto& RenderData = Renderable.GetRenderData();
		auto& Transform = RenderData.Transform;
		auto Position = Transform.GetPosition();
		if( ImGui::DragFloat3( "##mp", &Position.X, 0.1f ) ) //
		{
			Transform.SetPosition( Position );
		}

		auto Orientation = Transform.GetOrientation();
		if( ImGui::DragFloat3( "##mo", &Orientation.X ) )
		{
			Transform.SetOrientation( Orientation );
		}

		if( !Animation.Mesh )
			return;

		UI::AddAABB( Renderable.GetRenderData().WorldBounds.Minimum, Renderable.GetRenderData().WorldBounds.Maximum );

		const auto& Set = Animation.Mesh->GetAnimationSet();
		const auto& Skeleton = Set.Skeleton;
		if( Skeleton.Bones.empty() )
		{
			// Got no bones to pick.
			return;
		}

		if( Set.Skeleton.Animations.empty() )
		{
			ImGui::Text( "Renderable has no animations." );
			return;
		}

		if( ImGui::BeginCombo( "##AnimationAssets", Animation.CurrentAnimation.c_str() ) )
		{
			for( const auto& Pair : Set.Skeleton.Animations )
			{
				if( ImGui::Selectable( Pair.first.c_str() ) )
				{
					Animation.SetAnimation( Pair.first, LoopAnimation );
				}

				ImGui::Separator();
			}

			ImGui::EndCombo();
		}

		ImGui::Checkbox( "Loop##LoopAnimation", &LoopAnimation );
		ImGui::DragFloat( "Play Rate", &PlayRate, 0.01f );
	}

	const char* GetName() override
	{
		return MeshName.c_str();
	}

	const char* GetType() const override
	{
		return "Mesh";
	}

	CRenderable Renderable;
	Animator::Instance Animation;
	bool LoopAnimation = false;
	float PlayRate = 1.0f;

	std::string MeshName = std::string();
	std::string ShaderName = std::string();
	std::string TextureName = std::string();

	void Export( CData& Data ) const override
	{
		TrackEvent::Export( Data );

		Serialize::Export( Data, "msh", MeshName );
		Serialize::Export( Data, "shd", ShaderName );
		Serialize::Export( Data, "tex", TextureName );
		Serialize::Export( Data, "lpa", LoopAnimation );
		Serialize::Export( Data, "prt", PlayRate );
		Serialize::Export( Data, "anm", Animation.CurrentAnimation );
	}

	void Import( CData& Data ) override
	{
		TrackEvent::Import( Data );

		Serialize::Import( Data, "msh", MeshName );
		Serialize::Import( Data, "shd", ShaderName );
		Serialize::Import( Data, "tex", TextureName );
		Serialize::Import( Data, "lpa", LoopAnimation );
		Serialize::Import( Data, "prt", PlayRate );
		Serialize::Import( Data, "anm", Animation.CurrentAnimation );

		const auto& Assets = CAssets::Get();
		Renderable.SetMesh( Assets.Meshes.Find( MeshName ) );
		Renderable.SetShader( Assets.Shaders.Find( ShaderName ) );
		Renderable.SetTexture( Assets.Textures.Find( TextureName ), ETextureSlot::Slot0 );

		Animation.Mesh = Renderable.GetMesh();
		Animation.SetAnimation( Animation.CurrentAnimation, LoopAnimation );
	}
};

static std::map<std::string, std::function<TrackEvent* ( )>> EventTypes
{
	std::make_pair( "Audio", CreateTrack<FEventAudio> ),
	std::make_pair( "Camera", CreateTrack<FEventCamera> ),
	std::make_pair( "Mesh", CreateTrack<FEventRenderable> ),
	std::make_pair( "Image", CreateTrack<FEventImage> ),
	std::make_pair( "Entity", CreateTrack<EventEntity> )
};

void TrackEvent::Evaluate( const Timecode& Marker )
{
	if( InRange( Marker ) )
	{
		Execute();
	}
}

void TrackEvent::Context()
{
	ImGui::Text( "%s", GetName() );

	ImGui::Separator();
	int InputLength = StaticCast<int>( Length );
	if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
	{
		Length = static_cast<Timecode>( InputLength );
	}
}

void TrackEvent::AddType( const std::string& Name, const std::function<TrackEvent* ( )>& Generator )
{
	if( EventTypes.find( Name ) == EventTypes.end() )
	{
		EventTypes.insert_or_assign( Name, Generator );
	}
}

void TrackEvent::UpdateInternalMarkers( const Timecode& Marker )
{
	if( Timeline->Scrubbing )
	{
		PreviousOffset = 0;
	}
	else
	{
		PreviousOffset = Offset;
	}

	StoredMarker = Marker;
	Offset = Math::Clamp( Marker - Start, StaticCast<Timecode>( 0 ), Length );
}

void TrackEvent::Export( CData& Data ) const
{
	DataString::Encode( Data, GetType() );
	Data << Start;
	Data << Length;
}

void TrackEvent::Import( CData& Data )
{
	std::string Type;
	DataString::Decode( Data, Type );
	*this = *EventTypes[Type]();
	Data >> Start;
	Data >> Length;
}

void FTrack::AddEvent( TrackEvent* Event )
{
	Events.emplace_back( Event );
}

void FTrack::Evaluate( const Timecode& Marker )
{
	for( auto* Event : Events )
	{
		if( Event )
		{
			Event->UpdateInternalMarkers( Marker );
			Event->Evaluate( Marker );
		}
	}
}

void FTrack::Reset()
{
	for( auto* Event : Events )
	{
		if( Event )
		{
			Event->Reset();
		}
	}
}

CData& operator<<( CData& Data, const FTrack& Track )
{
	DataString::Encode( Data, "Track" );
	Data << Track.Start;
	Data << Track.Length;

	// DataVector::Encode( Data, Track.Events );
	DataString::Encode( Data, "Events" );
	uint32_t Events = Track.Events.size();
	Data << Events;

	for( const auto* Event : Track.Events )
	{
		DataString::Encode( Data, Event->GetType() );
		Data << Event;
	}

	return Data;
}

CData& operator>>( CData& Data, FTrack& Track )
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

		TrackEvent* Event = EventTypes[String]();
		Track.Events.emplace_back( Event );

		Data >> Event;
	}

	return Data;
}

CTimeline::CTimeline( Timecode Start, Timecode End )
{
	StartMarker = Start;
	EndMarker = End;
	Marker = 0;
	Status = SequenceStatus::Stopped;

	DrawTimeline = false;
}

bool CTimeline::Load( const char* FileLocation )
{
	// TODO: Figure out if we can remove this empty check so that timelines can be reloaded.
	/*if( !Tracks.empty() )
		return false;*/

	if( FileLocation )
	{
		Location = FileLocation;
	}

	CFile File( Location );
	if( !File.Exists() )
		return false;

	File.Load( true );
	File.Extract( *this );

	if( FileLocation )
	{
		Location = FileLocation;
	}

	return true;
}

void CTimeline::Save( const char* FileLocation ) const
{
	const char* SaveLocation = FileLocation ? FileLocation : Location.c_str();
	if( !SaveLocation )
		return;

	CData Data;
	Data << *this;

	CFile File( SaveLocation );
	File.Load( Data );
	File.Save();
}

void CTimeline::Play()
{
	if( Status != SequenceStatus::Paused )
	{
		Marker = StartMarker;

		for( auto& Track : Tracks )
		{
			Track.Reset();
			Track.Evaluate( Marker );
		}
	}

	Status = SequenceStatus::Playing;
	StartTime = GameLayersInstance->GetRealTime();
}

void CTimeline::Pause()
{
	if( Status == SequenceStatus::Paused || Status == SequenceStatus::Stopped )
	{
		Status = SequenceStatus::Playing;

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

	Status = SequenceStatus::Paused;

	for( auto& Track : Tracks )
	{
		Track.Reset();
	}
}

void CTimeline::Stop()
{
	Status = SequenceStatus::Stopped;
	Marker = StartMarker;

	for( auto& Track : Tracks )
	{
		Track.Reset();
	}
}

bool CTimeline::Playing() const
{
	return Status == SequenceStatus::Playing;
}

bool CTimeline::Stopped() const
{
	return Status == SequenceStatus::Stopped || Status == SequenceStatus::Paused;
}

void CTimeline::Step()
{
	if( Status < SequenceStatus::Playing )
		return;

	// Increment the timeline step.
	Marker++;

	// Check if we should use the loop points
	if( PlaybackMode == Repeat && LoopStart != InvalidTimecode && LoopEnd != InvalidTimecode )
	{
		if( Marker >= LoopEnd )
		{
			Marker = LoopStart;
		}
	}

	// Stop the timeline playback if we have crossed the end marker.
	if( Marker >= EndMarker )
	{
		if( PlaybackMode == Repeat )
		{
			Play();
		}
		else
		{
			Stop();
		}
	}
}

void CTimeline::GoTo( const Timecode& MarkerLocation )
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

Timecode CTimeline::CurrentMarker() const
{
	return Marker;
}

Timecode CTimeline::Size() const
{
	return StartMarker - EndMarker;
}

double CTimeline::Time() const
{
	return MarkerToTime( Marker );
}

double CTimeline::Length() const
{
	return MarkerRangeToTime( StartMarker, EndMarker );
}

void CTimeline::Feed()
{
	const auto CurrentTime = GameLayersInstance->GetCurrentTime();
	const auto TimeSinceStart = CurrentTime - StartTime;
	const auto Drift = Math::Max( 0.0, TimeSinceStart - Time() );
	const auto StepDrift = StaticCast<uint64_t>( std::ceil( Drift * StaticCast<double>( Timebase ) ) );
	const auto DeltaTime = CurrentTime - PreviousTime;
	auto Steps = StaticCast<uint64_t>( std::floor( DeltaTime * PlayRate * StaticCast<double>( Timebase ) ) );
	Steps += StepDrift;

	for( uint64_t StepIndex = 0; StepIndex < Steps; StepIndex++ )
	{
		if( Status == SequenceStatus::Stopped )
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

	PreviousTime = CurrentTime;
}

void CTimeline::Frame()
{
	// Clear the active timeline camera.
	ActiveCamera = nullptr;

	ConfigureEvents();

	// Run all the timeline events that are associated with the current marker.
	if( Status != SequenceStatus::Stopped )
	{
		for( auto& Track : Tracks )
		{
			Track.Evaluate( Marker );
		}
	}

	Scrubbing = false;

	// Make sure we clear the active camera.
	if( !ActiveCamera && Playing() )
	{
		auto* World = CWorld::GetPrimaryWorld();
		if( World && World->GetActiveCamera() == ActiveCamera )
		{
			World->SetActiveCamera( nullptr );
		}
	}

	if( DrawTimeline )
	{
		DisplayTimeline();
	}
	else
	{
		LastDrag = TrackState();
		AutoFit = false;
		SnapToHandles = false;
		Visualize = false;
		AutoScroll = true;
	}

	Feed();
}

void CTimeline::ConfigureEvents()
{
	size_t TrackIndex = 0;
	for( auto& Track : Tracks )
	{
		for( auto* Event : Track.Events )
		{
			Event->TrackIndex = TrackIndex;
			Event->Timeline = this;
		}

		TrackIndex++;
	}
}

void CTimeline::Draw()
{
	DrawTimeline = true;
}

void CTimeline::DisplayTimeline()
{
	const auto TimelineName = "Timeline: " + Location;
	if( ImGui::Begin(
		TimelineName.c_str(),
		&DrawTimeline,
		ImVec2( 1000.0f, 500.0f ), 0.45f
	) )
	{
		static int SequenceLengthSeconds = 2;

		// NOTE: This play/pause code isn't ideal because it doesn't always work. (an item has to be hovered) Plus it triggers when you're typing.
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

		if( ImGui::Button( "Reload" ) )
		{
			Load();
		}

		ImGui::SameLine();

		if( ImGui::Button( "Save" ) )
		{
			Save();
		}

		if( ImGui::Button( "Create Track" ) )
		{
			CreateTrack();
			ActiveTrack = nullptr;
			ActiveTrackIndex = InvalidTrackIndex;
		}

		ImGui::SameLine();

		static std::string EventType = ( *EventTypes.find( "Audio" ) ).first;

		if( ImGui::Button( "Create Event" ) )
		{
			if( ActiveTrack && !Tracks.empty() )
			{
				TrackEvent* Event = EventTypes[EventType]();
				Event->Start = Marker;
				Event->Length = SequenceLengthSeconds * Timebase;

				Event->TrackIndex = ActiveTrackIndex;
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

		ImGui::Checkbox( "Auto Fit", &AutoFit );

		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			ImGui::Text( "When enabled, the timeline will automatically update to fit all events." );
			ImGui::EndTooltip();
		}

		ImGui::SameLine();

		ImGui::Checkbox( "Visualize All", &Visualize );

		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			ImGui::Text( "Enables visualization for all events." );
			ImGui::EndTooltip();
		}

		ImGui::SameLine();
		ImGui::Checkbox( "Snap when Stretching", &SnapToHandles );

		ImGui::SameLine();
		ImGui::Checkbox( "Auto Scroll", &AutoScroll );

		ImGui::SameLine();
		if( ImGui::Button( "Scroll to Marker" ) )
		{
			ScrollMarker = InvalidTimecode;
		}

		ImGui::SameLine();
		if( ImGui::Button( "Set Loop Start" ) )
		{
			PlaybackMode = Repeat;
			LoopStart = Marker;

			if( LoopEnd == InvalidTimecode )
			{
				LoopEnd = EndMarker;
			}
		}

		ImGui::SameLine();
		if( ImGui::Button( "Set Loop End" ) )
		{
			PlaybackMode = Repeat;
			LoopEnd = Marker;

			if( LoopStart == InvalidTimecode )
			{
				LoopStart = StartMarker;
			}
		}

		ImGui::SameLine();
		if( ImGui::Button( "Clear Loop" ) )
		{
			LoopStart = InvalidTimecode;
			LoopEnd = InvalidTimecode;
			PlaybackMode = Once;
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth( 100.0f );

		int StartLocation = StartMarker / Timebase;
		int EndLocation = EndMarker / Timebase;
		/*if( ImGui::InputInt( "Start", &StartLocation ) )
			{
				StartMarker = StartLocation * Timebase;
			}*/

		if( ImGui::InputInt( "End", &EndLocation ) )
		{
			EndMarker = EndLocation * Timebase;
		}

		ImGui::Separator();
		ImGui::Separator();

		static float WidthScale = 1.0f;
		size_t WidthDelta = Math::Max( 1.0f, static_cast<size_t>( 1000.0f / 30 ) * WidthScale );
		ImGui::DragFloat( "Scale", &WidthScale, 0.1f, 0.1f, 100.0f );

		if( ImGui::GetIO().KeyCtrl )
		{
			WidthScale = Math::Max( 0.1f, WidthScale + ImGui::GetIO().MouseWheel * 0.33f );
		}

		ImGui::SameLine();
		if( ImGui::Checkbox( "Use Timeline Cameras", &EnableCamera ) )
		{
			if( !EnableCamera )
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

		if( AutoScroll || ScrollMarker == InvalidTimecode )
		{
			ScrollMarker = Marker;
		}

		auto ScrollMarkerPosition = ( static_cast<float>( ScrollMarker ) / static_cast<float>( Timebase ) ) * WidthDelta;

		const auto WindowWidth = ImGui::GetWindowWidth() - CursorOffset;
		const auto MarkerPercentageWindow = ScrollMarkerPosition / WindowWidth;
		const auto EndMarkerPercentageWindow = ScrollMarkerPosition / WindowWidth;

		if( Status != SequenceStatus::Stopped && EndMarkerPercentageWindow > 1.0f && MarkerPercentageWindow > 0.5f )
		{
			CursorOffset -= ScrollMarkerPosition - WindowWidth * 0.5f;
		}

		// UI::AddText( Vector2D( 50.0f, 130.0f ), "MarkerWindowPosition", MarkerWindowPosition );
		// UI::AddText( Vector2D( 50.0f, 150.0f ), "MarkerPercentageWindow", MarkerPercentageWindow );

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
		Timecode SnapMarker = 0;

		auto MousePosition = ImGui::GetMousePos();
		auto MouseOffset = MousePosition.x - MouseStartPosition.x;
		if( MouseOffset < 0.0f )
			MouseOffset = 0.0f;

		GhostMarker = static_cast<Timecode>( ( MouseOffset / WidthDelta ) * Timebase );
		SnapMarker = GhostMarker;

		if( ImGui::IsItemHovered() )
		{
			GhostMarker = static_cast<Timecode>( ( MouseOffset / WidthDelta ) * Timebase );
			if( ImGui::GetIO().MouseDown[0] )
			{
				GoTo( GhostMarker );
				Scrubbing = true;

				if( Status == SequenceStatus::Stopped )
				{
					Status = SequenceStatus::Paused;
				}
			}
			else if( ImGui::GetIO().MouseDown[1] )
			{
				int32_t ScrollMove = static_cast<int32_t>( GhostMarker ) - static_cast<int32_t>( ScrollMarker );
				ScrollMarker = ScrollMarker + ScrollMove / 30;
			}
		}

		ImGui::SetCursorPos( BarPosition );

		ImGui::Separator();

		CursorPosition = ImGui::GetCursorPos();
		auto ScreenPosition = ImGui::GetCursorScreenPos();

		struct EventTrackDrag
		{
			bool Up = false;
			size_t TrackIndex = 0;
			TrackEvent* Event = nullptr;
		};

		std::vector<EventTrackDrag> DragEvents;
		DragEvents.reserve( 2 );

		struct EventTrackStretch
		{
			int Left = 0;
			int Right = 0;
			size_t TrackIndex = 0;
			TrackEvent* Event = nullptr;
		};

		std::vector<EventTrackStretch> StretchEvents;
		StretchEvents.reserve( 2 );

		TrackState NewState;

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
				ActiveTrackIndex = TrackIndex;
			}

			// Select all events of a track when shift is held down.
			if( ImGui::IsItemClicked() && ImGui::GetIO().KeyShift )
			{
				ClearActiveEvents();
				for( auto* Event : Track.Events )
				{
					ActiveEvents.insert( Event );
				}
			}

			TrackPosition.x += CursorOffset;
			ImGui::SetCursorPosX( TrackPosition.x );

			size_t EventIndex = 0;
			for( auto* Event : Track.Events )
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

				const float Brightness = SelectedEvent ? 0.75f : 0.4f;
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
				ImVec2 EventSize = ImVec2( ( static_cast<float>( Event->Length ) / static_cast<float>( Timebase ) ) * WidthDelta, 30.0f );
				ImGui::Button( EventLabel.c_str(), EventSize );
				if( ImGui::IsItemHovered() )
				{
					ImGui::SetTooltip( EventName.c_str() );
				}

				char ContextName[128];
				sprintf_s( ContextName, "ec##%zi", EventIndex );
				if( ImGui::BeginPopupContextItem( ContextName ) )
				{
					const auto FrameHeight = ImGui::GetFrameHeight();
					const auto ButtonSize = ImVec2( FrameHeight, FrameHeight );
					if( ImGui::Button( "D##EventDuplicate", ButtonSize ) )
					{
						EventDuplicate( Event );
					}

					if( ImGui::IsItemHovered() )
					{
						ImGui::BeginTooltip();
						ImGui::Text( "Duplicate event" );
						ImGui::EndTooltip();
					}

					ImGui::SameLine();

					if( ImGui::Button( "S##EventSplit", ButtonSize ) )
					{
						EventSplit( Event );
					}

					if( ImGui::IsItemHovered() )
					{
						ImGui::BeginTooltip();
						ImGui::Text( "Split event" );
						ImGui::EndTooltip();
					}

					ImGui::SameLine();

					const std::string ArrowLabelDown = "##MoveDown" + std::to_string( EventIndex );
					if( ImGui::ArrowButton( ArrowLabelDown.c_str(), ImGuiDir_Down ) )
					{
						EventTrackDrag DragEvent;
						DragEvent.Up = false;
						DragEvent.TrackIndex = TrackIndex;
						DragEvent.Event = Event;

						DragEvents.emplace_back( DragEvent );
					}

					if( ImGui::IsItemHovered() )
					{
						ImGui::BeginTooltip();
						ImGui::Text( "Move down 1 track" );
						ImGui::EndTooltip();
					}

					ImGui::SameLine();

					const std::string ArrowLabelUp = "##MoveUp" + std::to_string( EventIndex );
					if( ImGui::ArrowButton( ArrowLabelUp.c_str(), ImGuiDir_Up ) )
					{
						EventTrackDrag DragEvent;
						DragEvent.Up = true;
						DragEvent.TrackIndex = TrackIndex;
						DragEvent.Event = Event;

						DragEvents.emplace_back( DragEvent );
					}

					if( ImGui::IsItemHovered() )
					{
						ImGui::BeginTooltip();
						ImGui::Text( "Move up 1 track" );
						ImGui::EndTooltip();
					}

					ImGui::SameLine();

					Event->Context();
					ImGui::EndPopup();
				}

				if( Visualize )
				{
					Event->Visualize();
				}

				if( ImGui::IsItemClicked() )
				{
					if( !LastDrag.Event && !ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift )
					{
						ClearActiveEvents();
					}

					const auto Iterator = ActiveEvents.find( Event );
					if( Iterator == ActiveEvents.end() )
					{
						ActiveTrack = &Track;
						ActiveTrackIndex = TrackIndex;
						ActiveEvents.insert( Event );
					}
					else if( ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeyShift )
					{
						ActiveEvents.erase( Iterator );
					}
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

				ImGui::SetCursorPos( EventPosition );

				const float HandleWidth = EventSize.x > 40.0f ? 20.0f : 5.0f;

				const std::string StretchHandleLeftLabel = "##StretchLeft" + std::to_string( GlobalIndex );
				ImGui::Button( StretchHandleLeftLabel.c_str(), ImVec2( HandleWidth, 30.0f ) );

				if( !Scrubbing && ( ImGui::IsItemHovered( ImGuiHoveredFlags_RectOnly ) || ImGui::IsItemClicked() ) )
				{
					ImGui::BeginTooltip();
					if( LastDrag.Event && LastDrag.Stretch && LastDrag.Left )
					{
						ImGui::Text( "<<<" );
					}
					else
					{
						ImGui::Text( "<" );
					}
					ImGui::EndTooltip();

					if( ImGui::GetIO().MouseDown[0] && !LastDrag.Event )
					{
						StretchEvents.emplace_back();
						auto& Stretch = StretchEvents.back();
						Stretch.Left = static_cast<int>( Drag.x / WidthDelta * Timebase );
						Stretch.Event = Event;
						Stretch.TrackIndex = TrackIndex;

						NewState.Event = Event;
						NewState.Stretch = true;
						NewState.Left = true;
						NewState.CursorPosition = ImGui::GetIO().MousePos;
						NewState.TrackIndex = TrackIndex;
					}

					Drag.x = 0.0f;
				}

				ImVec2 HandleRightPosition = EventPosition;
				HandleRightPosition.x += EventSize.x - HandleWidth;
				ImGui::SetCursorPos( HandleRightPosition );

				const std::string StretchHandleRightLabel = "##StretchRight" + std::to_string( GlobalIndex );
				ImGui::Button( StretchHandleRightLabel.c_str(), ImVec2( HandleWidth, 30.0f ) );
				if( !Scrubbing && ( ImGui::IsItemHovered( ImGuiHoveredFlags_RectOnly ) || ImGui::IsItemClicked() ) )
				{
					ImGui::BeginTooltip();
					if( LastDrag.Event && LastDrag.Stretch && !LastDrag.Left )
					{
						ImGui::Text( ">>>" );
					}
					else
					{
						ImGui::Text( ">" );
					}
					ImGui::EndTooltip();

					if( ImGui::GetIO().MouseDown[0] && !LastDrag.Event )
					{
						StretchEvents.emplace_back();
						auto& Stretch = StretchEvents.back();
						Stretch.Right = static_cast<int>( Drag.x / WidthDelta * Timebase );
						Stretch.Event = Event;
						Stretch.TrackIndex = TrackIndex;

						NewState.Event = Event;
						NewState.Stretch = true;
						NewState.Left = false;
						NewState.CursorPosition = ImGui::GetIO().MousePos;
						NewState.TrackIndex = TrackIndex;
					}

					Drag.x = 0.0f;
				}

				if( StretchEvents.empty() && !LastDrag.Event )
				{
					auto NewEventStart = Event->Start + static_cast<Timecode>( Drag.x / WidthDelta * Timebase );
					if( fabs( Drag.x ) > 0.001f && NewEventStart > Track.Start && NewEventStart < ( Track.Start + Track.Length ) )
					{
						/*Event->Start = NewEventStart;

							auto EventCode = Event->Start + Event->Length;
							auto TrackCode = Track.Start + Track.Length;
							if( EventCode > TrackCode )
							{
								Track.Length = EventCode - Track.Start;
							}*/

						NewState.Event = Event;
						NewState.Stretch = false;
						NewState.Left = false;
						NewState.CursorPosition = ImGui::GetIO().MousePos;
						NewState.TrackIndex = TrackIndex;
					}
				}

				// TODO: Disabled vertical dragging for now.
				//if( fabs( Drag.y ) > 1.0f && ImGui::IsItemActive() ) // 
				//{
				//	const bool Up = Drag.y < 0.0f;
				//	EventTrackDrag DragEvent;
				//	DragEvent.Up = Up;
				//	DragEvent.TrackIndex = TrackIndex;
				//	DragEvent.Event = Event;

				//	DragEvents.emplace_back( DragEvent );
				//}

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

		if( LastDrag.Event )
		{
			if( LastDrag.Stretch )
			{
				bool Snapped = false;
				TrackEvent* Closest = nullptr;
				int SnapBounds = 2 * Timebase;
				int ClosestDistance = SnapBounds;
				bool Flipped = false;
				for( auto& Track : Tracks )
				{
					for( auto* Event : Track.Events )
					{
						if( Event == LastDrag.Event )
							continue;

						if( LastDrag.Left )
						{
							int Distance = Event->Start;
							Distance -= LastDrag.Event->Start;
							Distance = std::abs( Distance );

							if( Distance < ClosestDistance )
							{
								ClosestDistance = Distance;
								Closest = Event;
								Flipped = false;
							}

							Distance = Event->Start;
							Distance -= LastDrag.Event->Start - LastDrag.Event->Length;
							Distance = std::abs( Distance );

							if( Distance < ClosestDistance )
							{
								// ClosestDistance = Distance;
								// Closest = Event;
								// Flipped = true;
							}
						}
						else
						{
							int Distance = Event->Start + Event->Length;
							Distance -= LastDrag.Event->Start + LastDrag.Event->Length;
							Distance = std::abs( Distance );

							if( Distance < ClosestDistance )
							{
								ClosestDistance = Distance;
								Closest = Event;
								Flipped = false;
							}

							Distance = Event->Start + Event->Length;
							Distance -= LastDrag.Event->Start;
							Distance = std::abs( Distance );

							if( Distance < ClosestDistance )
							{
								// ClosestDistance = Distance;
								// Closest = Event;
								// Flipped = true;
							}
						}
					}
				}

				if( Closest && SnapToHandles )
				{
					ImGui::BeginTooltip();
					ImGui::Text( "Closest event: %s", Closest->GetName() );

					if( LastDrag.Left )
					{
						int DistanceToMarker = LastDrag.Event->Start - GhostMarker;
						DistanceToMarker = abs( DistanceToMarker );
						if( DistanceToMarker < SnapBounds * 2 )
						{
							if( Flipped )
							{
								LastDrag.Event->Start = Closest->Start + Closest->Length;
								ImGui::Text( "FLIP AAH" );
							}
							else
							{
								LastDrag.Event->Start = Closest->Start;
							}

							Snapped = true;
						}
						else
						{
							if( Flipped )
							{
								SnapMarker = Closest->Start + Closest->Length;
							}
							else
							{
								SnapMarker = Closest->Start;
							}
						}
					}
					else
					{
						int DistanceToMarker = ( LastDrag.Event->Start + LastDrag.Event->Length ) - GhostMarker;
						DistanceToMarker = abs( DistanceToMarker );
						if( DistanceToMarker < SnapBounds * 2 )
						{
							int Delta = LastDrag.Event->Start - Closest->Start;
							LastDrag.Event->Length = Closest->Length - Delta;

							if( Flipped )
							{
								// LastDrag.Event->Length = Closest->Length - Delta;
							}
							else
							{
								LastDrag.Event->Length = Closest->Length - Delta;
							}

							Snapped = true;
						}
						else
						{
							SnapMarker = Closest->Start + Closest->Length;
						}

						ImGui::Text( "Distance to marker: %i", DistanceToMarker );
					}

					ImGui::EndTooltip();
				}

				if( !Snapped )
				{
					auto Drag = ImGui::GetIO().MousePos;
					Drag.x -= LastDrag.CursorPosition.x;
					Drag.y -= LastDrag.CursorPosition.y;

					LastDrag.CursorPosition = ImGui::GetIO().MousePos;

					StretchEvents.emplace_back();
					auto& Stretch = StretchEvents.back();
					if( LastDrag.Left )
					{
						Stretch.Left = static_cast<int>( Drag.x / WidthDelta * Timebase );
					}
					else
					{
						Stretch.Right = static_cast<int>( Drag.x / WidthDelta * Timebase );
					}

					Stretch.Event = LastDrag.Event;
					Stretch.TrackIndex = TrackIndex - 1;
				}
			}
			else
			{
				constexpr auto DragFactor = 1.0f;

				auto Drag = ImGui::GetIO().MousePos;
				Drag.x -= LastDrag.CursorPosition.x;
				Drag.y -= LastDrag.CursorPosition.y;

				Drag.x *= DragFactor;
				Drag.y *= DragFactor;

				const auto IsDragged = fabs( Drag.x ) > 0.001f;
				const auto DragOffset = static_cast<Timecode>( Drag.x / static_cast<float>( WidthDelta ) * Timebase );

				LastDrag.CursorPosition = ImGui::GetIO().MousePos;
				{
					auto* Event = LastDrag.Event;
					auto& Track = Tracks[LastDrag.TrackIndex];
					const auto NewEventStart = Event->Start + DragOffset;

					const auto LowerBound = NewEventStart > Track.Start;
					const auto UpperBound = NewEventStart < ( Track.Start + Track.Length );

					if( IsDragged && LowerBound && UpperBound )
					{
						Event->Start = NewEventStart;

						auto EventCode = Event->Start + Event->Length;
						auto TrackCode = Track.Start + Track.Length;
						if( EventCode > TrackCode )
						{
							Track.Length = EventCode - Track.Start;
						}

						if( EventCode > EndMarker )
						{
							EndMarker = EventCode;
						}
					}
				}

				for( auto* Event : ActiveEvents )
				{
					if( Event->TrackIndex == InvalidTrackIndex || Event == LastDrag.Event )
						continue;

					auto& Track = Tracks[Event->TrackIndex];
					const auto NewEventStart = Event->Start + DragOffset;

					const auto LowerBound = NewEventStart > Track.Start;
					const auto UpperBound = NewEventStart < ( Track.Start + Track.Length );

					if( IsDragged && LowerBound && UpperBound )
					{
						Event->Start = NewEventStart;

						auto EventCode = Event->Start + Event->Length;
						auto TrackCode = Track.Start + Track.Length;
						if( EventCode > TrackCode )
						{
							Track.Length = EventCode - Track.Start;
						}

						if( EventCode > EndMarker )
						{
							EndMarker = EventCode;
						}
					}
				}
			}
		}

		auto* DrawList = ImGui::IsWindowFocused() ? ImGui::GetOverlayDrawList() : ImGui::GetWindowDrawList();
		if( DrawList )
		{
			auto VerticalOffset = 32.0f;
			ScreenPosition.x += CursorOffset;

			auto MarkerPosition = ScreenPosition;
			MarkerPosition.x += MarkerLocalPosition;
			auto LineEnd = MarkerPosition;
			LineEnd.y += Tracks.size() * VerticalOffset;
			DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32_WHITE );

			// UI::AddText( Vector2D( 50.0f, 50.0f ), "ScreenPositionX", ScreenPosition.x );
			// UI::AddText( Vector2D( 50.0f, 70.0f ), "MarkerPosition", MarkerPosition.x );
			// UI::AddText( Vector2D( 50.0f, 90.0f ), "WindowWidth", WindowWidth );
			// UI::AddText( Vector2D( 50.0f, 110.0f ), "CursorPosition", CursorPosition.x );

			MarkerPosition = ScreenPosition;
			MarkerPosition.x += static_cast<float>( GhostMarker ) / static_cast<float>( Timebase ) * WidthDelta;
			LineEnd = MarkerPosition;
			LineEnd.y += Tracks.size() * VerticalOffset;
			DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32( 128, 128, 128, 128 ) );

			if( SnapMarker != GhostMarker )
			{
				MarkerPosition = ScreenPosition;
				MarkerPosition.x += static_cast<float>( SnapMarker ) / static_cast<float>( Timebase ) * WidthDelta;
				LineEnd = MarkerPosition;
				LineEnd.y += Tracks.size() * VerticalOffset;
				DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32( 255, 255, 0, 128 ) );
			}

			MarkerPosition = ScreenPosition;
			MarkerPosition.x += static_cast<float>( StartMarker ) / static_cast<float>( Timebase ) * WidthDelta;
			LineEnd = MarkerPosition;
			LineEnd.y += Tracks.size() * VerticalOffset;
			DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32_BLACK, 2 );

			MarkerPosition = ScreenPosition;
			MarkerPosition.x += static_cast<float>( EndMarker ) / static_cast<float>( Timebase ) * WidthDelta;
			LineEnd = MarkerPosition;
			LineEnd.y += Tracks.size() * VerticalOffset;
			DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32_BLACK, 2 );

			// Draw the loop points, when valid.
			if( LoopStart != InvalidTimecode )
			{
				MarkerPosition = ScreenPosition;
				MarkerPosition.x += static_cast<float>( LoopStart ) / static_cast<float>( Timebase ) * WidthDelta;
				LineEnd = MarkerPosition;
				LineEnd.y += Tracks.size() * VerticalOffset;
				DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32( 0, 255, 0, 128 ), 2 );
			}

			if( LoopEnd != InvalidTimecode )
			{
				MarkerPosition = ScreenPosition;
				MarkerPosition.x += static_cast<float>( LoopEnd ) / static_cast<float>( Timebase ) * WidthDelta;
				LineEnd = MarkerPosition;
				LineEnd.y += Tracks.size() * VerticalOffset;
				DrawList->AddLine( MarkerPosition, LineEnd, IM_COL32( 255, 0, 0, 128 ), 2 );
			}
		}

		for( auto& StretchEvent : StretchEvents )
		{
			auto* Event = StretchEvent.Event;
			auto& Track = Tracks[StretchEvent.TrackIndex];

			auto OffsetLeft = StretchEvent.Left;
			auto DistanceLeft = std::abs( OffsetLeft );
			if( OffsetLeft < 0 && DistanceLeft > Event->Start )
			{
				OffsetLeft = 0;
			}

			Event->Start += OffsetLeft;
			Event->Length -= OffsetLeft;

			auto OffsetRight = StretchEvent.Right;
			auto DistanceRight = std::abs( OffsetRight );
			if( OffsetRight < 0 && DistanceRight > Event->Length )
			{
				OffsetRight = 0;
			}

			Event->Length += OffsetRight;
			Event->Length = Math::Max( Timecode( 1 ), Event->Length );

			auto EventCode = Event->Start + Event->Length;
			if( EventCode > EndMarker )
			{
				EndMarker = EventCode;
			}
		}

		if( StretchEvents.empty() && !LastDrag.Event )
		{
			bool DragEventOccured = false;
			for( auto& DragEvent : DragEvents )
			{
				size_t NewTrackIndex = Math::Min( Tracks.size() - 1, Math::Max( size_t( 0 ), DragEvent.Up ? ( DragEvent.TrackIndex == 0 ? 0 : DragEvent.TrackIndex - 1 ) : DragEvent.TrackIndex + 1 ) );
				if( NewTrackIndex != DragEvent.TrackIndex )
				{
					auto Iterator = std::find( Tracks[DragEvent.TrackIndex].Events.begin(), Tracks[DragEvent.TrackIndex].Events.end(), DragEvent.Event );
					if( Iterator != Tracks[DragEvent.TrackIndex].Events.end() )
					{
						Tracks[DragEvent.TrackIndex].Events.erase( Iterator );
						Tracks[NewTrackIndex].Events.emplace_back( DragEvent.Event );
						DragEventOccured = true;
						ActiveTrack = &Tracks[NewTrackIndex];
						ActiveTrackIndex = NewTrackIndex;
					}
				}
			}
		}

		if( LastDrag.Event && !ImGui::GetIO().MouseDown[0] )
		{
			LastDrag = TrackState();
		}

		if( !LastDrag.Event )
		{
			LastDrag = NewState;
		}

		auto& Input = CInputLocator::Get();
		static bool DeleteState = false;
		bool WantsToDelete = ImGui::IsKeyReleased( ImGui::GetKeyIndex( ImGuiKey_Delete ) );

		const bool DeleteHeld = Input.IsKeyDown( EKey::Delete );
		if( DeleteHeld == false && DeleteState == true ) // Key released.
		{
			if( !ImGui::IsAnyItemActive() )
			{
				WantsToDelete = true;
				ImGui::GetIO().WantCaptureKeyboard = true;
			}
		}

		DeleteState = DeleteHeld;

		if( WantsToDelete )
		{
			DeleteActiveEvents();
		}

		// static bool DuplicateState = false;
		bool WantsToDuplicate = false;
		const auto KeyCode = InputGLFW::KeyToCode[static_cast<size_t>( EKey::D )];
		const bool DuplicateReleased = ImGui::GetIO().KeyCtrl && ImGui::IsKeyReleased( KeyCode );
		if( DuplicateReleased )
		{
			if( !ImGui::IsAnyItemActive() )
			{
				WantsToDuplicate = true;
			}
		}

		//const bool DuplicateHeld = Input.IsKeyDown( EKey::LeftControl ) && Input.IsKeyDown( EKey::D );
		//if( DuplicateHeld == false && DuplicateState == true ) // Key released.
		//{
		//	WantsToDuplicate = true;
		//}

		//DuplicateState = DuplicateHeld;

		if( WantsToDuplicate )
		{
			if( ActiveTrack && !Tracks.empty() )
			{
				for( auto& ActiveEvent : ActiveEvents )
				{
					if( ActiveEvent )
					{
						EventDuplicate( ActiveEvent );
					}
				}

				ClearActiveEvents();
			}
		}

		Timecode NewEndMarker = 0;

		// Update the length of each track.
		for( auto& Track : Tracks )
		{
			Track.Length = EndMarker - StartMarker;

			for( auto* Event : Track.Events )
			{
				Timecode EventCode = Event->Start + Event->Length;
				Timecode TrackCode = Track.Start + Track.Length;
				if( EventCode > TrackCode )
				{
					Track.Length = EventCode - Track.Start;
				}

				if( EventCode > NewEndMarker )
				{
					NewEndMarker = EventCode;

					if( AutoFit )
					{
						EndMarker = NewEndMarker;
					}
				}
			}
		}

		// Enable visualization just for active events when it's off.
		if( !Visualize )
		{
			for( auto* Event : ActiveEvents )
			{
				if( !Event )
					continue;

				Event->Visualize();
			}
		}
	}

	ImGui::End();
}

void CTimeline::CreateTrack()
{
	FTrack Track;
	Track.Start = StartMarker;
	Track.Length = EndMarker - StartMarker;

	Tracks.emplace_back( Track );
}

void CTimeline::AdjustTrack( TrackEvent* Event )
{
	const auto EventCode = Event->Start + Event->Length;
	const auto TrackCode = ActiveTrack->Start + ActiveTrack->Length;
	if( EventCode > TrackCode )
	{
		ActiveTrack->Length = EventCode - ActiveTrack->Start;
	}

	const auto AdjustedEnd = ActiveTrack->Start + ActiveTrack->Length;
	if( AdjustedEnd > EndMarker )
	{
		EndMarker = AdjustedEnd;
	}
}

void CTimeline::EventDuplicate( TrackEvent* Source )
{
	if( !ActiveTrack )
		return;

	CData Data;
	Source->Export( Data );

	TrackEvent* Event = EventTypes[Source->GetType()]();
	Event->Import( Data );

	Event->Start = Source->Start + Source->Length;
	Event->Length = Source->Length;

	Event->TrackIndex = ActiveTrackIndex;
	ActiveTrack->AddEvent( Event );

	AdjustTrack( Event );
}

void CTimeline::EventSplit( TrackEvent* Source )
{
	if( !ActiveTrack )
		return;

	CData Data;
	Source->Export( Data );

	TrackEvent* Event = EventTypes[Source->GetType()]();
	Event->Import( Data );

	Event->Start = Source->Start + Source->Length / 2;
	Event->Length = Source->Length / 2;

	Event->TrackIndex = ActiveTrackIndex;
	ActiveTrack->AddEvent( Event );

	Source->Length /= 2;

	AdjustTrack( Event );
}

void CTimeline::EventSplitAtMarker( TrackEvent* Source )
{
	if( !ActiveTrack )
		return;

	if( Marker < Source->Start )
		return;

	const Timecode SplitOffset = Marker - Source->Start;
	const Timecode SplitRemainder = Source->Length - SplitOffset;

	CData Data;
	Source->Export( Data );

	TrackEvent* Event = EventTypes[Source->GetType()]();
	Event->Import( Data );

	Event->Start = Source->Start + SplitOffset;
	Event->Length = SplitRemainder;

	Event->TrackIndex = ActiveTrackIndex;
	ActiveTrack->AddEvent( Event );

	Source->Length = SplitOffset;

	AdjustTrack( Event );
}

void CTimeline::Stretch( const float& Factor )
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

void CTimeline::DeleteActiveEvents()
{
	for( auto* ActiveEvent : ActiveEvents )
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

	ClearActiveEvents();
}

void CTimeline::ClearActiveEvents()
{
	ActiveEvents.clear();
}

CData& operator<<( CData& Data, const CTimeline& Timeline )
{
	DataString::Encode( Data, Timeline.Location );

	Data << Timeline.StartMarker;
	Data << Timeline.EndMarker;

	DataVector::Encode( Data, Timeline.Tracks );

	return Data;
}

CData& operator>>( CData& Data, CTimeline& Timeline )
{
	DataString::Decode( Data, Timeline.Location );

	Data >> Timeline.StartMarker;
	Data >> Timeline.EndMarker;

	Timeline.Tracks.clear();

	DataVector::Decode( Data, Timeline.Tracks );

	return Data;
}