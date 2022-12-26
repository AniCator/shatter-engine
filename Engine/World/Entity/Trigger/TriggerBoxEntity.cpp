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
		Volume->SetBounds( { Vector3D( -1.0f ), Vector3D( 1.0f ) } );

		Volume->OnEnter = [this] ( Interactable* Interactable )
		{
			OnEnter( Interactable );
		};

		Volume->OnLeave = [this] ( Interactable* Interactable )
		{
			OnLeave( Interactable );
		};

		Volume->Condition = [this] ( Interactable* Interactable )
		{
			if( !IsValidEntity( Interactable ) )
				return false;

			return true;
		};
	}

	if( const auto* World = GetWorld() )
	{
		Filter = World->Find( FilterName );
	}

	Tag( "trigger" );
}

void CTriggerBoxEntity::Tick()
{
	if( !CanTrigger() )
	{
		if( !Latched )
			return;

		// Check if any of the entities just left.
		for( auto* Entity : LatchedEntities )
		{
			const auto* MeshEntity = Cast<CMeshEntity>( Entity );
			if( MeshEntity && !Volume->WorldBounds.Intersects( MeshEntity->GetWorldBounds() ) )
			{
				// Send( "OnLeave" );
				break;
			}
		}
		
		Latched = false;
		LatchedEntities.clear();
		return;
	}

	if( !ShouldTrigger() )
		return;

	Send( "OnTrigger" );
	Latched = true;

	Count++;

	// Check if any of the entities just entered.
	for( auto* Entity : Volume->Entities )
	{
		if( LatchedEntities.find( Entity ) == LatchedEntities.end() )
		{
			// Send( "OnEnter" );
			break;
		}
	}

	// Copy the volume's entity set.
	for( auto* Entity : Volume->Entities )
	{
		if( Filter && Entity )
		{
			const auto* MeshEntity = Cast<CMeshEntity>( Entity );
			if( MeshEntity && MeshEntity == Filter && Volume->WorldBounds.Intersects( MeshEntity->GetWorldBounds() ) )
			{
				LatchedEntities.insert( Entity );
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

void CTriggerBoxEntity::Debug()
{
	CPointEntity::Debug();

	Volume->Debug();

	UI::AddAABB( 
		Transform.GetPosition() + Bounds.Minimum, 
		Transform.GetPosition() + Bounds.Maximum, 
		Latched ? Color::Green : Color::Red 
	);

	std::string EntityString;
	for( auto* Interactable : Volume->Entities )
	{
		EntityString += (dynamic_cast<CEntity*>( Interactable ))->Name.String() + "\n";
	}

	if( Volume->Entities.empty() )
	{
		EntityString = "(none)\n";
	}

	if( FilterName.length() > 0 )
	{
		EntityString += "Filter: " + FilterName + "\n";
	}

	UI::AddText( Transform.GetPosition() + Bounds.Maximum, EntityString.c_str() );
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

void CTriggerBoxEntity::OnEnter( Interactable* Interactable )
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

void CTriggerBoxEntity::OnLeave( Interactable* Interactable )
{
	if( IsDebugEnabled() )
	{
		Log::Event( "OnLeave\n" );
	}

	Send( "OnLeave", this );
}

const std::unordered_set<Interactable*>& CTriggerBoxEntity::Fetch() const
{
	return Volume->Entities;
}

bool CTriggerBoxEntity::ShouldTrigger() const
{
	return !Latched && ( Frequency < 0 || Count < Frequency );
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
			if( IsValidEntity( Interactable ) )
			{
				// We've found a match.
				return true;
			}
		}

		// No match was found.
		return false;
	}

	// No filter was specified and any interactable will cause this trigger to fire.
	return FilterName.empty();
}

bool CTriggerBoxEntity::IsValidEntity( Interactable* Interactable ) const
{
	const auto* Entity = Cast<CEntity>( Interactable );
	if( Entity && Entity == Filter )
	{
		return true;
	}

	return FilterName.empty();
}
