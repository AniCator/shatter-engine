// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TriggerProximityEntity.h"

#include <Engine/World/World.h>

static CEntityFactory<CTriggerProximityEntity> Factory( "trigger_proximity" );

CTriggerProximityEntity::CTriggerProximityEntity()
{
	Latched = false;
	Frequency = -1;
	TriggerCount = 0;
}

void CTriggerProximityEntity::Construct()
{
	
}

void CTriggerProximityEntity::Tick()
{
	auto World = GetWorld();
	if( World )
	{
		const auto& CameraSetup = World->GetActiveCameraSetup();
		Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

		float Length = Delta.Length();
		if( Length < Radius )
		{
			if( !Latched && ( Frequency < 0 || TriggerCount < Frequency ) )
			{
				Send( "OnTrigger" );
				Latched = true;

				TriggerCount++;
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
}

void CTriggerProximityEntity::Destroy()
{

}

void CTriggerProximityEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	for( auto Property : Objects )
	{
		if( Property->Key == "radius" )
		{
			size_t OutTokenCount = 0;
			auto TokenDistance = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, 1 );
			if( OutTokenCount == 1 )
			{
				Radius = TokenDistance[0];
			}
		}
		else if( Property->Key == "frequency" )
		{
			const double PropertyFrequency = ParseDouble( Property->Value.c_str() );
			if( PropertyFrequency > 0.0f )
			{
				Frequency = static_cast<int32_t>( PropertyFrequency );
			}
		}
	}
}

void CTriggerProximityEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << Radius;
	Data << Frequency;
	Data << Latched;
	Data << TriggerCount;
}

void CTriggerProximityEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> Radius;
	Data >> Frequency;
	Data >> Latched;
	Data >> TriggerCount;
}
