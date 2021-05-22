// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MoveLinear.h"

#include <Game/Game.h>
#include <Engine/World/World.h>

static CEntityFactory<CMoveLinearEntity> Factory( "move_linear" );

CMoveLinearEntity::CMoveLinearEntity() : CMeshEntity()
{
	Start = Vector3D( 0.0f, 0.0f, 0.0f );
	Direction = Vector3D( 0.0f, 0.0f, 1.0f );
	Distance = 0.0f;
	Speed = 0.0f;
	MoveTo = true;

	Static = false;
	Stationary = true;
}

void CMoveLinearEntity::Construct()
{
	Start = Transform.GetPosition();

	CMeshEntity::Construct();
}

void CMoveLinearEntity::Tick()
{
	if( MoveTo )
	{
		Vector3D Position = Transform.GetPosition();
		Vector3D Delta = Start - Position;
		float DistanceToEnd = Delta.Length();
		if( DistanceToEnd > Distance )
		{
			MoveTo = false;
		}
		else
		{
			Position += Direction * Speed * GameLayersInstance->GetDeltaTime();
			Transform.SetPosition( Position );
		}
	}
	else
	{
		Vector3D End = Start + Direction * Distance;
		Vector3D Position = Transform.GetPosition();
		Vector3D Delta = End - Position;
		float DistanceToStart = Delta.Length();
		if( DistanceToStart > Distance )
		{
			MoveTo = true;
		}
		else
		{
			Position -= Direction * Speed * GameLayersInstance->GetDeltaTime();
			Transform.SetPosition( Position );
		}
	}

	SetTransform( Transform );

	CMeshEntity::Tick();
}

void CMoveLinearEntity::Destroy()
{
	CMeshEntity::Destroy();
}

void CMoveLinearEntity::Load( const JSON::Vector& Objects )
{
	CMeshEntity::Load( Objects );

	for( auto Property : Objects )
	{
		if( Property->Key == "distance" )
		{
			Extract(Property->Value.c_str(), Distance );
		}
		else if( Property->Key == "speed" )
		{
			Extract( Property->Value.c_str(), Speed );
		}
		else if( Property->Key == "direction" )
		{
			Extract( Property->Value.c_str(), Direction );
			Direction.Normalize();
		}
	}
}

void CMoveLinearEntity::Export( CData& Data )
{
	CMeshEntity::Export( Data );
	Data << Start;
	Data << Direction;
	Data << Distance;
	Data << Speed;
	Data << MoveTo;
}

void CMoveLinearEntity::Import( CData& Data )
{
	CMeshEntity::Import( Data );
	Data >> Start;
	Data >> Direction;
	Data >> Distance;
	Data >> Speed;
	Data >> MoveTo;
}
