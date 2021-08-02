// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TriggerBoxEntity.h"

#include <Engine/World/World.h>

static CEntityFactory<CTriggerBoxEntity> Factory( "trigger_box" );

void CTriggerBoxEntity::Construct()
{
	if( !Volume )
	{
		Volume = new CTriggerBody<Interactable*>( this );
		Volume->Construct();

		const auto Size = Vector3D( 1.0f );
		Volume->SetBounds( FBounds( -Size, Size ) );
	}
}

void CTriggerBoxEntity::Tick()
{
	auto* World = GetWorld();
	if( World )
	{
		const auto& CameraSetup = World->GetActiveCameraSetup();
		const auto Delta = CameraSetup.CameraPosition - Transform.GetPosition();

		float Length = Delta.Length();
		if( !Volume->Entities.empty() )
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
}

void CTriggerBoxEntity::Destroy()
{
	if( Volume )
	{
		Volume->Destroy();
		delete Volume;
	}
}

void CTriggerBoxEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	for( const auto* Property : Objects )
	{
		if( Property->Key == "frequency" )
		{
			const double PropertyFrequency = ParseDouble( Property->Value.c_str() );
			if( PropertyFrequency > 0.0f )
			{
				Frequency = static_cast<int32_t>( PropertyFrequency );
			}
		}
	}
}

void CTriggerBoxEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << Frequency;
	Data << Latched;
	Data << Count;
}

void CTriggerBoxEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> Frequency;
	Data >> Latched;
	Data >> Count;
}
