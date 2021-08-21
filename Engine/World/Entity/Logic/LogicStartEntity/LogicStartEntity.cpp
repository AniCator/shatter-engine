// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LogicStartEntity.h"

static CEntityFactory<CLogicStartEntity> Factory( "logic_start" );

CLogicStartEntity::CLogicStartEntity()
{
	HasStarted = false;
}

void CLogicStartEntity::Construct()
{
	
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
	
}

void CLogicStartEntity::Export( CData& Data )
{
	Data << HasStarted;
}

void CLogicStartEntity::Import( CData& Data )
{
	Data >> HasStarted;
}
