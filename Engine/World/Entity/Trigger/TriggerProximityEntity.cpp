// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TriggerProximityEntity.h"

#include <Engine/Display/UserInterface.h>
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

	if( !Volume )
	{
		Volume = new CTriggerBody<Interactable*>( this );
		Volume->Construct();
		Volume->SetBounds( BoundingBox( Transform.GetPosition() - Radius, Transform.GetPosition() + Radius ) );
	}
}

void CTriggerProximityEntity::Tick()
{
	auto* World = GetWorld();
	if( !World )
		return;

	if( CanTrigger() )
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
	CPointEntity::Destroy();

	if( Volume )
	{
		Volume->Destroy();
		delete Volume;
	}
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

void CTriggerProximityEntity::Debug()
{
	CPointEntity::Debug();

	UI::AddSphere( Transform.GetPosition(), Radius, Color::Red );
}

// TODO: Add support for a filter like in trigger_box.
bool CTriggerProximityEntity::CanTrigger() const
{
	// Check if any interactables are within the volume.
	const bool IsEmpty = Volume->Entities.empty();
	if( IsEmpty )
		return false;

	// Check if any of the entities are within the trigger's actual radius.
	const auto RadiusSquared = Radius * Radius;
	for( auto* Entity : Volume->Entities )
	{
		if( auto* PointEntity = Cast<CPointEntity>( Entity ) )
		{
			const auto Delta = PointEntity->GetTransform().GetPosition() - Transform.GetPosition();
			if( Delta.LengthSquared() < RadiusSquared )
			{
				// This entity is within range.
				return true;
			}
		}
	}

	return false;
}
