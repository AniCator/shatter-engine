// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "StartEntity.h"

static CEntityFactory<CLogicStartEntity> Factory( "logic_start" );

CLogicStartEntity::CLogicStartEntity()
{
	Construct();
}

void CLogicStartEntity::Construct()
{
	HasStarted = false;
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
