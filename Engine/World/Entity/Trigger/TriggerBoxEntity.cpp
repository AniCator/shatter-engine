// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TriggerBoxEntity.h"

#include <Engine/World/World.h>

static CEntityFactory<CTriggerBoxEntity> Factory( "trigger_box" );

CTriggerBoxEntity::CTriggerBoxEntity()
{
	Inputs["Trigger"] = [&] ( CEntity* Origin )
	{
		Send( "OnTrigger" );

		return true;
	};
}

void CTriggerBoxEntity::Construct()
{
	if( !Volume )
	{
		Volume = new CTriggerBody<Interactable*>( this );
		Volume->Construct();
		Volume->SetBounds( Bounds );
	}

	Tag( "trigger" );
}

void CTriggerBoxEntity::Tick()
{
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

void CTriggerBoxEntity::Destroy()
{
	if( Volume )
	{
		Volume->Destroy();
		delete Volume;
	}

	CPointEntity::Destroy();
}

void CTriggerBoxEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	for( const auto* Property : Objects )
	{
		if( Property->Key == "frequency" )
		{
			const double PropertyFrequency = ParseDouble( Property->Value.c_str() );
			if( PropertyFrequency > 0.0 )
			{
				Frequency = static_cast<int32_t>( PropertyFrequency );
			}
		}
		else if( Property->Key == "filter" )
		{
			FilterName = Property->Value;
		}
		else if( Property->Key == "bounds" )
		{
			// The bounds are defined as two vectors separated by a comma.
			const auto Vectors = ExtractTokens( Property->Value.c_str(), ',', 2 );
			if( Vectors.size() == 2 )
			{
				Extract( Vectors[0].c_str(), Bounds.Minimum );
				Extract( Vectors[1].c_str(), Bounds.Maximum );
			}
		}
	}
}

void CTriggerBoxEntity::Reload()
{
	const auto* World = GetWorld();
	if( !World )
		return;
	
	Filter = World->Find( FilterName );
	
	CPointEntity::Reload();
}

void CTriggerBoxEntity::Debug()
{
	CPointEntity::Debug();

	UI::AddAABB( 
		Transform.GetPosition() + Bounds.Minimum, 
		Transform.GetPosition() + Bounds.Maximum, 
		Latched ? Color::Green : Color::Red 
	);
}

void CTriggerBoxEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );
	Data << Frequency;
	Data << Latched;
	Data << Count;
	Data << Bounds;
	
	DataString::Encode( Data, FilterName );
}

void CTriggerBoxEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );
	Data >> Frequency;
	Data >> Latched;
	Data >> Count;
	Data >> Bounds;

	DataString::Decode( Data, FilterName );
}

bool CTriggerBoxEntity::CanTrigger() const
{
	// Check if any interactables are within the volume.
	const bool IsEmpty = Volume->Entities.empty();
	if( IsEmpty )
		return false;
	
	const bool UseFilter = Filter != nullptr;
	if( UseFilter )
	{
		// Check if the filtered entity is among the interactables.
		for( auto* Interactable : Volume->Entities )
		{
			if( Cast<CEntity>( Interactable ) == Filter )
			{
				// We've found a match.
				return true;
			}
		}

		// No match was found.
		return false;
	}

	// No filter was specified and any interactable will cause this trigger to fire.
	return true;
}
