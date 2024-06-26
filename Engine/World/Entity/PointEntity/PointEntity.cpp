// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "PointEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

#include <Engine/Display/UserInterface.h>

CPointEntity::CPointEntity()
{
	Transform = FTransform();
}

CPointEntity::CPointEntity( const FTransform& Transform ) : CEntity()
{
	this->Transform = Transform;
	ShouldUpdateTransform = true;
	PreviousWorldTransform = Transform;
}

CPointEntity::~CPointEntity()
{

}

void CPointEntity::Tick()
{
	// Update the absolute velocity based on the difference between the previous world transform and the current world transform.
	Velocity = GetTransform().GetPosition() - PreviousWorldTransform.GetPosition();
	PreviousWorldTransform = WorldTransform;
}

const FTransform& CPointEntity::GetTransform()
{
	if( ShouldUpdateTransform )
	{
		Transform.Update();

		if( Level && false )
		{
			WorldTransform = Level->GetTransform().Transform( Transform );
		}
		else
		{
			WorldTransform = Transform;
		}

		if( Parent )
		{
			auto Entity = dynamic_cast<CPointEntity*>( Parent );
			if( Entity )
			{
				auto ParentTransform = Entity->GetTransform();
				WorldTransform = ParentTransform.Transform( WorldTransform );
			}
		}

		ShouldUpdateTransform = false;
	}

	return WorldTransform;
}

const FTransform& CPointEntity::GetLocalTransform() const
{
	return Transform;
}

void CPointEntity::SetTransform( const FTransform& TransformIn )
{
#if _DEBUG
	const auto InPosition = TransformIn.GetPosition();
	const auto InOrientation = TransformIn.GetOrientation();
	if( !InPosition.IsValid() || !InOrientation.IsValid() )
	{
		Log::Event( Log::Warning, "Invalid transform.\n" );
	}
#endif

	Transform = TransformIn;
	ShouldUpdateTransform = true;

	// PreviousWorldTransform = Transform;
}

void CPointEntity::Load( const JSON::Vector& Objects )
{
	CAssets& Assets = CAssets::Get();

	Transform.Update();

	for( auto* Property : Objects )
	{
		if( Property->Key == "parent" )
		{
			ParentName = Property->Value;
		}
		else if( Property->Key == "position" )
		{
			Vector3D Position( 0.0f, 0.0f, 0.0f );
			Extract( Property->Value, Position );
			Transform.SetPosition( Position );
		}
		else if( Property->Key == "rotation" )
		{
			Vector3D Orientation( 0.0f, 0.0f, 0.0f );
			Extract( Property->Value, Orientation );
			Transform.SetOrientation( Orientation );
		}
		else if( Property->Key == "scale" )
		{
			Vector3D Size( 1.0f, 1.0f, 1.0f );
			Extract( Property->Value, Size );
			Transform.SetSize( Size );
		}
		else if( Property->Key == "tags" )
		{
			std::string Data;
			for( const auto& Character : Property->Value )
			{
				if( Character != ';' )
				{
					Data += Character;
				}
				else
				{
					Tag( Data );
				}
			}

			if( !Data.empty() )
			{
				Tag( Data );
			}
		}
		else if( Property->Key == "disable" )
		{
			Extract( Property->Value, Enabled );
			Enabled = !Enabled; // Flip it, since we're extracting a disabled state.
		}
	}

	ShouldUpdateTransform = true;

	if( Level && Transform.IsDirty() )
	{
		Transform = Level->GetTransform().Transform( Transform );
	}

	GetTransform();

	PreviousWorldTransform = Transform;
}

void CPointEntity::Debug()
{
	UI::AddCircle( WorldTransform.GetPosition(), 2.0f, Color::White );
	UI::AddText( WorldTransform.GetPosition() - Vector3D( 0.0, 0.0, -1.0f ), Name.String().c_str() );
	// UI::AddText( Transform.GetPosition() - Vector3D( 0.0, 0.0, 0.5f ), "Velocity", Velocity, Color::Blue );
}

Vector3D CPointEntity::GetVelocity() const
{
	return Velocity;
}

void CPointEntity::Import( CData& Data )
{
	Data >> Transform;
	Data >> WorldTransform;

	ShouldUpdateTransform = true;
	PreviousWorldTransform = WorldTransform;
}

void CPointEntity::Export( CData& Data )
{
	Data << Transform;
	Data << WorldTransform;
}
