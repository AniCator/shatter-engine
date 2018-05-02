// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include "ServiceRegistry.h"

#include <Engine/Input/Input.h>
#include <Engine/Utility/Locator/InputLocator.h>

CServiceRegistry::CServiceRegistry()
{
	EngineServices.reserve( 32 );
}

CServiceRegistry::~CServiceRegistry()
{
	for( size_t ServiceIndex = 0; ServiceIndex < EngineServices.size(); ServiceIndex++ )
	{
		delete EngineServices[ServiceIndex];
		EngineServices[ServiceIndex] = nullptr;
	}

	EngineServices.clear();
}

void CServiceRegistry::Add( IEngineService* Service )
{
	if( Service )
	{
		EngineServices.push_back( Service );
	}
}

void CServiceRegistry::CreateStandardServices()
{
	CInput* InputService = new CInput();
	CInputLocator::Assign( InputService );
	Add( InputService );
}
