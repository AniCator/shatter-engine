// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "EntityEvent.h"

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Timeline.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Gizmo.h>

#include <ThirdParty/imgui-1.70/imgui.h>

DragTransform DragA;
DragTransform DragB;

void EventEntity::Evaluate( const Timecode& Marker )
{
	TrackEvent::Evaluate( Marker );

	if( !Entity )
		return;

	// Check if the event has ended.
	if( HasEnded( Marker ) )
	{
		Entity->UsedBySequence = false;
	}

	if( !InRange( Marker ) )
		return;

	// As long as we're in range we want to mark the entity as "in use".
	Entity->UsedBySequence = true;
}

void EventEntity::Execute()
{
	if( !Entity )
	{
		FindEntities();
	}

	if( UseRecording )
	{
		PlayRecording();
		return;
	}

	FTransform Transform;
	if( !UseTransform )
	{
		if( !TargetEntity )
		{
			FindEntities();
		}
		else
		{
			TransformA = TargetEntity->GetTransform();
		}

		Transform = TransformA;
	}
	else
	{
		if( InterpolateLinear )
		{
			const auto Factor = static_cast<float>( MarkerToTime( Offset ) / MarkerToTime( Length ) );
			Transform = TransformA.Lerp( TransformB, Factor );
		}
		else
		{
			Transform = TransformA;
		}
	}

	if( Entity )
	{
		if( UseTransform )
		{
			Entity->SetTransform( Transform );

			if( auto* Body = Entity->GetBody() )
			{
				Body->PreviousTransform = Transform;
				Body->Acceleration = Vector3D::Zero;
				Body->Velocity = Vector3D::Zero;
			}
		}

		if( OverrideAnimation )
		{
			if( Entity->GetAnimation() != Animation )
			{
				Entity->SetAnimation( Animation, LoopAnimation );
			}

			// Calculate the animation time relative to the event.
			const auto MarkerTime = StoredMarker - Start;
			const auto AnimationTime = MarkerToTime( MarkerTime ) * PlayRate;

			// Apply the event-relative animation time.
			Entity->SetAnimationTime( static_cast<float>( AnimationTime ) );
			Entity->SetPlayRate( 0.0f );
		}
	}

	// A little hacky but this lets you move the transform point.
	if( UseTransform && DisplayGizmo )
	{
		PointGizmo( &TransformA, DragA );
		PointGizmo( &TransformB, DragB );
	}
}

void EventEntity::Reset()
{
	if( Entity )
	{
		Entity->UsedBySequence = false;
	}

	Entity = nullptr;
	TargetEntity = nullptr;
}

FTransform CopiedTransform;
void EventEntity::Context()
{
	ImGui::Text( "Entity Manipulation" );

	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "This event can be used to force mesh entities to a specific location." );
		ImGui::EndTooltip();
	}

	ImGui::Separator();
	int InputLength = StaticCast<int>( Length );
	if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
	{
		Length = static_cast<Timecode>( InputLength );
	}

	ImGui::Separator();

	if( ImGui::InputText( "Entity##Name", Name, 2048 ) )
	{
		UpdateEntities = true;
	}

	ImGui::Checkbox( "Replay", &UseRecording );
	ImGui::SameLine();

	if( Recording.Taped() )
	{
		ImGui::Text( "(%u samples)", Recording.Samples.size() );
	}
	else
	{
		ImGui::Text( "(no samples)" );
	}

	if( UseRecording )
	{
		ReplayOperation();
	}
	else
	{
		StandardOperation();
	}

	if( UpdateEntities )
	{
		FindEntities();
		UpdateEntities = false;
	}
}

void EventEntity::Visualize()
{
	if( !Record )
		return;

	if( !Entity )
	{
		FindEntities();
	}
	
	PerformRecording();
}

const char* EventEntity::GetName()
{
	return Name;
}

const char* EventEntity::GetType() const
{
	return "Entity";
}

void EventEntity::Export( CData& Data ) const
{
	TrackEvent::Export( Data );

	const std::string NameString = Name;
	Serialize::Export( Data, "nm", NameString );

	const std::string TargetString = Target;
	Serialize::Export( Data, "tg", TargetString );

	Serialize::Export( Data, "tb", UseTransform );
	Serialize::Export( Data, "tlrp", InterpolateLinear );
	Serialize::Export( Data, "tm", TransformA );
	Serialize::Export( Data, "tmb", TransformB );

	Serialize::Export( Data, "ab", OverrideAnimation );
	Serialize::Export( Data, "am", Animation );
	Serialize::Export( Data, "apr", PlayRate );
	Serialize::Export( Data, "alp", LoopAnimation );
	Serialize::Export( Data, "rec", UseRecording );

	Serialize::Export( Data, "smp", Recording );
}

void EventEntity::Import( CData& Data )
{
	TrackEvent::Import( Data );

	std::string NameString;
	Serialize::Import( Data, "nm", NameString );
	Serialize::StringToArray( NameString, Name, 2048 );

	std::string TargetString;
	Serialize::Import( Data, "tg", TargetString );
	Serialize::StringToArray( TargetString, Target, 2048 );

	Serialize::Import( Data, "tb", UseTransform );
	Serialize::Import( Data, "tlrp", InterpolateLinear );
	Serialize::Import( Data, "tm", TransformA );
	Serialize::Import( Data, "tmb", TransformB );

	Serialize::Import( Data, "ab", OverrideAnimation );
	Serialize::Import( Data, "am", Animation );
	Serialize::Import( Data, "apr", PlayRate );
	Serialize::Import( Data, "alp", LoopAnimation );
	Serialize::Import( Data, "rec", UseRecording );

	if( Serialize::Import( Data, "smp", Recording ) )
	{
		LookupAnimationStack = true;
	}
}

const Recording& EventEntity::GetRecording() const
{
	return Recording;
}

void EventEntity::FindEntities()
{
	auto* World = CWorld::GetPrimaryWorld();
	if( World )
	{
		auto* MeshSearch = Cast<CMeshEntity>( World->Find( Name ) );
		if( MeshSearch )
		{
			Entity = MeshSearch;
		}
		else
		{
			Entity = nullptr;
		}

		auto* Search = Cast<CPointEntity>( World->Find( Target ) );
		if( Search )
		{
			TargetEntity = Search;
		}
		else
		{
			TargetEntity = nullptr;
		}
	}
}

void EventEntity::StandardOperation()
{
	ImGui::Checkbox( "Use Transform", &UseTransform );
	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "When enabled, use the entity transform. Otherwise, use a target entity." );
		ImGui::EndTooltip();
	}

	if( ImGui::Checkbox( "Linear Interpolation", &InterpolateLinear ) )
	{
		TransformB = TransformA;
	}

	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "Interpolate between two transforms, clicking the checkbox copies the first transform to the second." );
		ImGui::EndTooltip();
	}

	if( ImGui::Button( "Copy##CopyA" ) )
	{
		CopiedTransform = TransformA;
	}

	ImGui::SameLine();
	if( ImGui::Button( "Paste##PasteA" ) )
	{
		TransformA = CopiedTransform;
	}

	if( UseTransform )
	{
		bool ShouldUpdate = false;
		auto Position = TransformA.GetPosition();
		if( ImGui::DragFloat3( "Position##mp", &Position.X, 0.1f ) )
		{
			TransformA.SetPosition( Position );
			ShouldUpdate = true;
		}

		auto Orientation = TransformA.GetOrientation();
		if( ImGui::DragFloat3( "Orientation##mo", &Orientation.X ) )
		{
			TransformA.SetOrientation( Orientation );
			ShouldUpdate = true;
		}

		if( ShouldUpdate )
		{
			TransformA.Update();
		}

		if( InterpolateLinear )
		{
			if( ImGui::Button( "Copy##CopyB" ) )
			{
				CopiedTransform = TransformB;
			}

			ImGui::SameLine();
			if( ImGui::Button( "Paste##PasteB" ) )
			{
				TransformB = CopiedTransform;
			}

			ShouldUpdate = false;
			Position = TransformB.GetPosition();
			if( ImGui::DragFloat3( "Position##mpb", &Position.X, 0.1f ) )
			{
				TransformB.SetPosition( Position );
				ShouldUpdate = true;
			}

			Orientation = TransformB.GetOrientation();
			if( ImGui::DragFloat3( "Orientation##mob", &Orientation.X ) )
			{
				TransformB.SetOrientation( Orientation );
				ShouldUpdate = true;
			}

			if( ShouldUpdate )
			{
				TransformB.Update();
			}
		}

		ImGui::Checkbox( "Display Gizmo", &DisplayGizmo );
		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			ImGui::Text( "Allows you to move the transform using a gizmo." );
			ImGui::EndTooltip();
		}
	}
	else
	{
		if( ImGui::InputText( "Target Entity##TargetName", Target, 2048 ) )
		{
			UpdateEntities = true;
		}
	}

	ImGui::Checkbox( "Override Animation", &OverrideAnimation );
	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "Forces the mesh entity's animation to the event's animation." );
		ImGui::EndTooltip();
	}

	if( OverrideAnimation )
	{
		if( Entity && Entity->Mesh )
		{
			const auto& Set = Entity->Mesh->GetAnimationSet();

			if( ImGui::BeginCombo( "##AnimationAssets", Animation.c_str() ) )
			{
				for( const auto& Pair : Set.Skeleton.Animations )
				{
					if( ImGui::Selectable( Pair.first.c_str() ) )
					{
						Animation = Pair.first;
					}

					ImGui::Separator();
				}

				ImGui::EndCombo();
			}

			if( Set.Skeleton.Animations.empty() )
			{
				ImGui::Text( "Entity has no animations." );
			}

			ImGui::Checkbox( "Loop##LoopAnimation", &LoopAnimation );
			ImGui::DragFloat( "Play Rate", &PlayRate, 0.01f );
		}
		else
		{
			ImGui::Text( "Can't configure an animation for an entity that doesn't exist." );
			UpdateEntities = true;
		}
	}
}

void EventEntity::ReplayOperation()
{
	const char* Label = Record ? "Stop" : "Record";
	if( ImGui::Button( Label ) )
	{
		Record = !Record;

		if( Record )
		{
			ClearRecording();

			// Insert ourselves as an active event to ensure recording works.
			Timeline->ActiveEvents.insert( this );
		}
	}

	if( ImGui::IsItemHovered() )
	{
		if( Record )
		{
			ImGui::BeginTooltip();
			ImGui::Text( "Stop recording animation data" );
			ImGui::EndTooltip();
		}
		else
		{
			ImGui::BeginTooltip();
			ImGui::Text( "Start recording animation data (overwrites existing recording)" );
			ImGui::EndTooltip();
		}
	}

	ImGui::SameLine();

	if( ImGui::Button( "Clear" ) )
	{
		ClearRecording();
	}

	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "Clear all recorded animation data for this event" );
		ImGui::EndTooltip();
	}
}

void EventEntity::PlayRecording()
{
	if( Record )
		return; // We're recording right now.

	if( !Entity )
		return; // We need an entity to operate on.

	const auto MarkerTime = StoredMarker - Start;
	const auto Time = MarkerToTime( MarkerTime );
	const auto Duration = MarkerToTime( Length );

	// Apply the sample.
	Recording.Apply( Entity, Time );
}

void EventEntity::PerformRecording()
{
	// Ensure recording keeps functioning since it currently relies on the Visualize function.
	Timeline->ActiveEvents.insert( this );

	if( !Entity )
	{
		Record = false;
		return; // No entity to record.
	}

	Recording::Sample Sample;
	Sample.Time = CEntity::GetCurrentTime(); // Record the game time.
	Sample.Transform = Entity->GetTransform();
	
	const auto& Instance = Entity->GetAnimationInstance();
	for( size_t Index = 0; Index < MaximumRecordingStackSize; Index++ )
	{
		if( Index >= Instance.Stack.size() )
			break; // The entity's animation instance stack has no more entries.

		Sample.Stack[Index] = Instance.Stack[Index];
		Sample.Entries++;
	}

	Recording.Samples.emplace_back( Sample );
}

void EventEntity::ClearRecording()
{
	Recording.Wipe();
}
