// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PerformanceStringTest.h"

#include <Profiling/Logging.h>
#include <Profiling/Profiling.h>

#include <Utility/StringPool.h>

#include <fstream>
#include <sstream>
#include <unordered_map>

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
	std::unordered_map<std::string, std::string> TestMap;

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
			TestPool.Create( Line );
		}

		CreatePoolTimer.Stop();

		CTimer PollPoolTimer( false );
		PollPoolTimer.Start();

		StringSymbol_t Symbol = TestPool.Find( std::string( TestString ) );

		PollPoolTimer.Stop();

		if( Symbol == CStringPool::InvalidSymbol )
		{
			Log::Event( "Pool: Test string not found.\n" );
		}
		else
		{
			Log::Event( "Pool: Test string found: %s\n", TestPool.Get( Symbol ).c_str() );
		}

		CTimer GetPoolTimer( false );
		GetPoolTimer.Start();

		const std::string& GetString = TestPool.Get( Symbol );

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
			TestMap.insert_or_assign( LineB, LineB );
		}

		CreateMapTimer.Stop();

		CTimer PollMapTimer( false );
		PollMapTimer.Start();

		auto Result = TestMap.find( TestString );

		PollMapTimer.Stop();

		if( Result == TestMap.end() )
		{
			Log::Event( "Map: Test string not found.\n" );
		}
		else
		{
			Log::Event( "Map: Test string found: %s\n", TestMap[TestString].c_str() );
		}

		CTimer GetMapTimer( false );
		GetMapTimer.Start();

		const std::string& GetString2 = TestMap[TestString];

		GetMapTimer.Stop();

		GetString2.c_str();

		Log::Event( "Map: Direct lookup time: %ius\n", GetMapTimer.GetElapsedTimeMicroseconds() );

		Log::Event( "Map creation time: %ius | Map poll time: %ius\n", CreateMapTimer.GetElapsedTimeMicroseconds(), PollMapTimer.GetElapsedTimeMicroseconds() );

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
