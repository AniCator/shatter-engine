// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "EntityEvent.h"

#include <Engine/Display/Rendering/Mesh.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>
#include <Engine/Utility/Gizmo.h>

#include <ThirdParty/imgui-1.70/imgui.h>

DragTransform Drag;

void EventEntity::Execute()
{
	if( !Entity )
	{
		FindEntities();
	}

	if( !UseTransform )
	{
		if( !TargetEntity )
		{
			FindEntities();
		}
		else
		{
			Transform = TargetEntity->GetTransform();
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

		if( OverrideAnimation && Entity->GetAnimation() != Animation )
		{
			Entity->SetAnimation( Animation );
		}
	}

	// A little hacky but this lets you move the transform point.
	if( UseTransform && MoveTransform )
	{
		PointGizmo( &Transform, Drag );
	}
}

void EventEntity::Reset()
{
	
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

	if( UseTransform )
	{
		auto Position = Transform.GetPosition();
		if( ImGui::InputFloat3( "##mp", &Position.X, "%.2f" ) )
		{
			Transform.SetPosition( Position );
		}

		auto Orientation = Transform.GetOrientation();
		if( ImGui::InputFloat3( "##mo", &Orientation.X, "%.2f" ) )
		{
			Transform.SetOrientation( Orientation );
		}

		ImGui::Checkbox( "Display Gizmo", &MoveTransform );
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
	Serialize::Export( Data, "tm", Transform );

	Serialize::Export( Data, "ab", OverrideAnimation );
	Serialize::Export( Data, "am", Animation );
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
	Serialize::Import( Data, "tm", Transform );

	Serialize::Import( Data, "ab", OverrideAnimation );
	Serialize::Import( Data, "am", Animation );
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
