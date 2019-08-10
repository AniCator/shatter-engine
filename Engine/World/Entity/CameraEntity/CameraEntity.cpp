// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "CameraEntity.h"

#include <Engine/World/World.h>

static CEntityFactory<CCameraEntity> Factory( "camera" );

CCameraEntity::CCameraEntity()
{
	Priority = 501;
}

void CCameraEntity::Construct()
{

}

void CCameraEntity::Tick()
{
	auto World = GetWorld();
	if( Active && World )
	{
		const FTransform& Transform = GetTransform();
		FCameraSetup& CameraSetup = Camera.GetCameraSetup();
		CameraSetup.CameraPosition = Transform.GetPosition();
		Camera.SetCameraOrientation( Transform.GetOrientation() );

		World->SetActiveCamera( &Camera, Priority );
	}
}

void CCameraEntity::Destroy()
{
	Deactivate();
}

void CCameraEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	FCameraSetup& CameraSetup = Camera.GetCameraSetup();
	for( auto& Property : Objects )
	{
		if( Property->Key == "fov" )
		{
			const double PropertyFOV = ParseDouble( Property->Value.c_str() );
			if( PropertyFOV > 0.0f )
			{
				CameraSetup.FieldOfView = PropertyFOV;
			}
		}
		else if( Property->Key == "priority" )
		{
			Priority = atoi( Property->Value.c_str() );
		}
	}
}

void CCameraEntity::Activate()
{
	Active = true;
}

void CCameraEntity::Deactivate()
{
	if( Active )
	{
		Active = false;

		auto World = GetWorld();
		if( World )
		{
			if( World->GetActiveCamera() == &Camera )
			{
				World->SetActiveCamera( nullptr, Priority );
			}
		}
	}
}

void CCameraEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << Camera;
	Data << Priority;
}

void CCameraEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> Camera;
	Data >> Priority;

	Camera.Update();
}
