// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "TestResult.h"

class CTest
{
public:
	CTest()
	{

	};

	virtual ~CTest()
	{

	};

	virtual ETestResult Run()
	{
		return ETestResult::Unknown;
	};

	virtual const char* GetName()
	{
		return "Unnamed";
	};
};
