// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Utility/Service/ServiceRegistry.h>

class CApplication
{
public:
	CApplication();
	~CApplication();

	void Run();

private:
	void Initialize();

	CServiceRegistry ServiceRegistry;
};
