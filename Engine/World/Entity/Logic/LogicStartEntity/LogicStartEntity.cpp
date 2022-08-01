// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LogicStartEntity.h"

static CEntityFactory<CLogicStartEntity> Factory( "logic_start" );

CLogicStartEntity::CLogicStartEntity()
{
	HasStarted = false;
}

void CLogicStartEntity::Construct()
{
	if( TriggerAlways )
	{
		HasStarted = false;
	}
}

void CLogicStartEntity::Tick()
{
	if( !HasStarted )
	{
		HasStarted = true;
		Send( "OnStart" );
	}
}

void CLogicStartEntity::Destroy()
{
	
}

void CLogicStartEntity::Load( const JSON::Vector& Objects )
{
	JSON::Assign( Objects, "always", TriggerAlways );
}

void CLogicStartEntity::Export( CData& Data )
{
	Data << HasStarted;
	Data << TriggerAlways;
}

void CLogicStartEntity::Import( CData& Data )
{
	Data >> HasStarted;
	Data >> TriggerAlways;
}
