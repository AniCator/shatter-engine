// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "CameraEntity.h"

#include <Engine/World/World.h>

static CEntityFactory<CCameraEntity> Factory( "camera" );

CCameraEntity::CCameraEntity()
{
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

		World->SetActiveCamera( &Camera );
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
				World->SetActiveCamera( nullptr );
			}
		}
	}
}
