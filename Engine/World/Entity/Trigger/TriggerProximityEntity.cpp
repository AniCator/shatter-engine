// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TriggerProximityEntity.h"

#include <Engine/World/World.h>

static CEntityFactory<CTriggerProximityEntity> Factory( "trigger_proximity" );

CTriggerProximityEntity::CTriggerProximityEntity()
{
	Inputs["Trigger"] = [&] ( CEntity* Origin )
	{
		Send( "OnTrigger" );

		return true;
	};
}

void CTriggerProximityEntity::Construct()
{
	Tag( "trigger" );
}

void CTriggerProximityEntity::Tick()
{
	auto* World = GetWorld();
	if( !World )
		return;

	const auto& CameraSetup = World->GetActiveCameraSetup();
	const Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

	const float Length = Delta.Length();
	if( Length < Radius )
	{
		if( !Latched && ( Frequency < 0 || Count < Frequency ) )
		{
			Send( "OnTrigger" );
			Latched = true;

			Count++;
		}
	}
	else
	{
		if( Latched )
		{
			Latched = false;
		}
	}
}

void CTriggerProximityEntity::Destroy()
{

}

void CTriggerProximityEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	for( const auto* Property : Objects )
	{
		if( Property->Key == "radius" )
		{
			Extract( Property->Value, Radius );
		}
		else if( Property->Key == "frequency" )
		{
			Extract( Property->Value, Frequency );
		}
	}
}

void CTriggerProximityEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << Radius;
	Data << Frequency;
	Data << Latched;
	Data << Count;
}

void CTriggerProximityEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> Radius;
	Data >> Frequency;
	Data >> Latched;
	Data >> Count;
}
