// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Service.h"
#include <vector>

class CServiceRegistry
{
public:
	CServiceRegistry();
	~CServiceRegistry();

	void Add( IEngineService* Service );

	void CreateStandardServices();

private:
	std::vector<IEngineService*> EngineServices;
};
