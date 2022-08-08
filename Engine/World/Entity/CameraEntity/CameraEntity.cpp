// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "CameraEntity.h"

#include <Engine/World/World.h>

static CEntityFactory<CCameraEntity> Factory( "camera" );

CCameraEntity::CCameraEntity()
{
	Inputs["Activate"] = [&] ( CEntity* Origin )
	{
		Activate();

		return true;
	};

	Inputs["Deactivate"] = [&] ( CEntity* Origin )
	{
		Deactivate();

		return true;
	};
}

void CCameraEntity::Construct()
{

}

void CCameraEntity::Tick()
{
	if( !Active )
		return;

	auto* World = GetWorld();
	if( !World )
		return;
	
	const FTransform& Transform = GetTransform();
	FCameraSetup& CameraSetup = Camera.GetCameraSetup();
	CameraSetup.CameraPosition = Transform.GetPosition();
	Camera.SetCameraOrientation( Transform.GetOrientation() );

	World->SetActiveCamera( &Camera, Priority );
}

void CCameraEntity::Destroy()
{
	Deactivate();
}

void CCameraEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	FCameraSetup& CameraSetup = Camera.GetCameraSetup();

	JSON::Assign( Objects, {
		{"fov", CameraSetup.FieldOfView},
		{"priority", Priority},
		} 
	);

	if( CameraSetup.FieldOfView <= 0.0f)
	{
		CameraSetup.FieldOfView = 60.0f;
	}
}

void CCameraEntity::Activate()
{
	Active = true;
}

void CCameraEntity::Deactivate()
{
	if( !Active )
		return;

	Active = false;

	auto* World = GetWorld();
	if( !World )
		return;

	if( World->GetActiveCamera() != &Camera )
		return;

	World->SetActiveCamera( nullptr, Priority );
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
