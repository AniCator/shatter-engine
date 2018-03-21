// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PerformanceStringTest.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Utility/StringPool.h>

#include <fstream>
#include <sstream>
#include <unordered_set>

#pragma optimize("", off)

CStringPerformanceTest::CStringPerformanceTest()
{

}

CStringPerformanceTest::~CStringPerformanceTest()
{

}

static const char* TestString = "Nicholas was spending the last of his leave at home. A fourth letter";

ETestResult CStringPerformanceTest::Run()
{
	CStringPool TestPool;
	std::unordered_set<std::string> TestSet;

	std::ifstream BigTextStream;
	BigTextStream.open( "big.txt" );
	if( !BigTextStream.fail() )
	{
		Log::Event( "Loading big.txt test file.\n" );

		Log::Event( "Test String: %s\n", TestString );

		CTimer CreatePoolTimer( false );
		CreatePoolTimer.Start();

		std::string Line;
		while( std::getline( BigTextStream, Line ) )
		{
			TestPool.Find( Line, true );
		}

		CreatePoolTimer.Stop();

		CTimer PollPoolTimer( false );
		PollPoolTimer.Start();

		auto& Symbol = TestPool.Find( std::string( TestString ) );

		PollPoolTimer.Stop();

		if( Symbol == std::string( "BADSTRING" ) )
		{
			Log::Event( "Pool: Test string not found.\n" );
		}
		else
		{
			Log::Event( "Pool: Test string found: %s\n", Symbol.c_str() );
		}

		CTimer GetPoolTimer( false );
		GetPoolTimer.Start();

		const std::string& GetString = TestPool.Find( std::string( TestString ) );

		GetPoolTimer.Stop();

		GetString.c_str();

		Log::Event( "Pool: Direct lookup time: %ius\n", GetPoolTimer.GetElapsedTimeMicroseconds() );

		Log::Event( "Pool creation time: %ius | Pool poll time: %ius\n", CreatePoolTimer.GetElapsedTimeMicroseconds(), PollPoolTimer.GetElapsedTimeMicroseconds() );

		BigTextStream.close();

		std::ifstream BigTextStream2;
		BigTextStream2.open( "big.txt" );

		CTimer CreateMapTimer( false );
		CreateMapTimer.Start();

		std::string LineB;
		while( std::getline( BigTextStream2, LineB ) )
		{
			TestSet.insert( LineB );
		}

		CreateMapTimer.Stop();

		CTimer PollMapTimer( false );
		PollMapTimer.Start();

		auto Result = TestSet.find( TestString );

		PollMapTimer.Stop();

		if( Result == TestSet.end() )
		{
			Log::Event( "Set: Test string not found.\n" );
		}
		else
		{
			const std::string ResultString = *Result;
			Log::Event( "Set: Test string found: %s\n", ResultString.c_str() );
		}

		CTimer GetMapTimer( false );
		GetMapTimer.Start();

		const std::string& GetString2 = *TestSet.find( TestString );

		GetMapTimer.Stop();

		GetString2.c_str();

		Log::Event( "Set: Direct lookup time: %ius\n", GetMapTimer.GetElapsedTimeMicroseconds() );

		Log::Event( "Set creation time: %ius | Map poll time: %ius\n", CreateMapTimer.GetElapsedTimeMicroseconds(), PollMapTimer.GetElapsedTimeMicroseconds() );

		return ETestResult::Succeeded;
	}
	else
	{
		Log::Event( "big.txt not found, test aborted.\n" );

		return ETestResult::Failed;
	}
}

const char* CStringPerformanceTest::GetName()
{
	return "String Performance Test";
}

CStringPerformanceTest StringPerformanceTest = CStringPerformanceTest();
