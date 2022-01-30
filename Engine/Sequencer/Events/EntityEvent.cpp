// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "EntityEvent.h"

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Gizmo.h>

#include <ThirdParty/imgui-1.70/imgui.h>

DragTransform DragA;
DragTransform DragB;

void EventEntity::Evaluate( const Timecode& Marker )
{
	StoredMarker = Marker;
	TrackEvent::Evaluate( Marker );
}

void EventEntity::Execute()
{
	if( !Entity )
	{
		FindEntities();
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
		Entity->SetTransform( Transform );

		if( auto* Body = Entity->GetBody() )
		{
			Body->PreviousTransform = Transform;
			Body->Acceleration = Vector3D::Zero;
			Body->Velocity = Vector3D::Zero;
		}

		if( OverrideAnimation )
		{
			if( Entity->GetAnimation() != Animation )
			{
				Entity->SetAnimation( Animation, LoopAnimation );
			}

			// Calculate the animation time relative to the event.
			const auto MarkerTime = StoredMarker - Start;
			const auto AnimationTime = MarkerToTime( MarkerTime );

			// Apply the event-relative animation time.
			Entity->SetAnimationTime( static_cast<float>( AnimationTime ) );
			Entity->SetPlayRate( PlayRate );
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
	Entity = nullptr;
	TargetEntity = nullptr;
}

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

	bool UpdateEntities = false;
	if( ImGui::InputText( "Entity##Name", Name, 2048 ) )
	{
		UpdateEntities = true;
	}

	ImGui::Checkbox( "Use Transform", &UseTransform );
	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "When enabled, use the entity transform. Otherwise, use a target entity." );
		ImGui::EndTooltip();
	}

	ImGui::Checkbox( "Linear Interpolation", &InterpolateLinear );
	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "Interpolate between two transforms." );
		ImGui::EndTooltip();
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
			ImGui::DragFloat( "Play Rate", &PlayRate, 0.1f, 0.1f );
		}
		else
		{
			ImGui::Text( "Can't configure an animation for an entity that doesn't exist." );
			UpdateEntities = true;
		}
	}

	if( UpdateEntities )
	{
		FindEntities();
	}
}

const char* EventEntity::GetName()
{
	return Name;
}

const char* EventEntity::GetType()
{
	return "Entity";
}

void EventEntity::Export( CData& Data )
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
