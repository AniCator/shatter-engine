#pragma once

#include "../Test.h"

class CStringPerformanceTest : CTest
{
public:
	CStringPerformanceTest();

	~CStringPerformanceTest();

	virtual ETestResult Run() override;
	virtual const char* GetName() override;
};
