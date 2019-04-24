// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "StartEntity.h"

static CEntityFactory<CStartEntity> Factory( "logic_start" );

CStartEntity::CStartEntity()
{
	Construct();
}

void CStartEntity::Construct()
{
	HasStarted = false;
}

void CStartEntity::Tick()
{
	if( !HasStarted )
	{
		HasStarted = true;
		Send( "OnStart" );
	}
}

void CStartEntity::Destroy()
{
	
}

void CStartEntity::Load( const JSON::Vector& Objects )
{
	
}
