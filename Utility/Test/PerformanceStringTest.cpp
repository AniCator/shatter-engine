// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PerformanceStringTest.h"

#include <Profiling/Logging.h>
#include <Profiling/Profiling.h>

CStringPerformanceTest::CStringPerformanceTest()
{

}

CStringPerformanceTest::~CStringPerformanceTest()
{

}

ETestResult CStringPerformanceTest::Run()
{
	CTimer Timer( false );
	Timer.Start();

	// Test area

	Timer.Stop();

	Log::Event( "Test time: %ius\n", Timer.GetElapsedTimeMicroseconds() );

	return ETestResult::Succeeded;
}

const char* CStringPerformanceTest::GetName()
{
	return "String Performance Test";
}

CStringPerformanceTest StringPerformanceTest = CStringPerformanceTest();
