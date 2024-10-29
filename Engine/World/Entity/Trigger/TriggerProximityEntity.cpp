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
	RadiusSquared = Radius * Radius;

	if( !Volume )
	{
		Volume = new CTriggerBody<Interactable*>( this );
		Volume->Construct();
		Volume->SetBounds( BoundingBox( Transform.GetPosition() - Radius, Transform.GetPosition() + Radius ) );

		Volume->OnEnter = [this]( Interactable* Interactable )
		{
			OnEnter( Interactable );
		};

		Volume->OnLeave = [this]( Interactable* Interactable )
		{
			OnLeave( Interactable );
		};

		Volume->Condition = [this]( Interactable* Interactable )
		{
			if( !IsValidEntity( Interactable ) )
				return false;

			// Check if we're actuall within the trigger radius, the trigger body volume works like a box by default.
			if( auto* PointEntity = Cast<CPointEntity>( Interactable ) )
			{
				const auto DistanceSquared = PointEntity->GetTransform().GetPosition().DistanceSquared( Transform.GetPosition() );
				if( DistanceSquared > RadiusSquared )
				{
					// This entity is out of range.
					return false;
				}
			}

			return true;
		};
	}

	if( const auto* World = GetWorld() )
	{
		Filter = World->Find( FilterName );
	}

	Tag( "trigger" );
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
		else if( Property->Key == "filter" )
		{
			FilterName = Property->Value;
		}
	}
}

void CTriggerProximityEntity::Debug()
{
	CPointEntity::Debug();

	UI::AddSphere( Transform.GetPosition(), Radius, Color::Red );

	std::string EntityString;
	for( auto* Interactable : Volume->Entities )
	{
		EntityString += ( dynamic_cast<CEntity*>( Interactable ) )->Name.String() + "\n";
	}

	if( Volume->Entities.empty() )
	{
		EntityString = "(none)\n";
	}

	if( FilterName.length() > 0 )
	{
		EntityString += "Filter: " + FilterName + "\n";
	}

	if( !Volume )
		return;

	UI::AddText( Transform.GetPosition() + Volume->GetWorldBounds().Maximum, EntityString.c_str() );
}

void CTriggerProximityEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << Radius;
	Data << Frequency;
	Data << Latched;
	Data << Count;

	DataString::Encode( Data, FilterName );
}

void CTriggerProximityEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> Radius;
	Data >> Frequency;
	Data >> Latched;
	Data >> Count;

	DataString::Decode( Data, FilterName );
}

void CTriggerProximityEntity::OnEnter( Interactable* Interactable )
{
	const auto ShouldTrigger = ( Frequency < 0 || Count < Frequency );
	if( !ShouldTrigger )
		return;

	if( IsDebugEnabled() )
	{
		Log::Event( "OnEnter\n" );
	}

	Send( "OnEnter", this );
}

void CTriggerProximityEntity::OnLeave( Interactable* Interactable )
{
	if( IsDebugEnabled() )
	{
		Log::Event( "OnLeave\n" );
	}

	Send( "OnLeave", this );
}

const std::unordered_set<Interactable*>& CTriggerProximityEntity::Fetch() const
{
	return Volume->Entities;
}

// TODO: Add support for a filter like in trigger_box.
bool CTriggerProximityEntity::CanTrigger() const
{
	// Check if any interactables are within the volume.
	return !Volume->Entities.empty();
}

bool CTriggerProximityEntity::IsValidEntity( Interactable* Interactable ) const
{
	const auto* Entity = Cast<CEntity>( Interactable );
	if( Entity && Entity == Filter )
	{
		return true;
	}

	return FilterName.empty();
}
