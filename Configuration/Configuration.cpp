// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Configuration.h"
#include <Profiling/Logging.h>
#include <Profiling/Profiling.h>

#include <fstream>
#include <sstream>

bool CConfiguration::IsValidKey( const char* KeyName )
{
	if( StoredSettings.find( KeyName ) == StoredSettings.end() )
	{
		Log::Event( "Invalid key \"%s\"\n", KeyName );
		return false;
	}

	return true;
}

bool CConfiguration::IsEnabled( const char* KeyName )
{
	std::string Value = GetValue( KeyName );

	if( Value.compare( "1" ) == 0 || Value.compare( "true" ) == 0 )
	{
		return true;
	}

	return false;
}

const char* CConfiguration::GetString( const char* KeyName )
{
	if( !IsValidKey( KeyName ) )
	{
		return "undefined";
	}

	return GetValue( KeyName ).c_str();
}

int CConfiguration::GetInteger( const char* KeyName )
{
	if( !IsValidKey( KeyName ) )
	{
		return 0;
	}

	return atoi( StoredSettings[KeyName].c_str() );
}

double CConfiguration::GetDouble( const char* KeyName )
{
	if( !IsValidKey( KeyName ) )
	{
		return 0.0;
	}

	return atof( StoredSettings[KeyName].c_str() );
}

float CConfiguration::GetFloat( const char* KeyName )
{
	return static_cast<float>( GetDouble( KeyName ) );
}

void CConfiguration::Initialize( std::string FilePath )
{
	ProfileBare( __FUNCTION__ );

	Log::Event( "Loading configuration.\n" );

	FileName = FilePath;
	std::ifstream configurationFileStream;
	configurationFileStream.open( FileName.c_str(), std::ios::binary );
	if( configurationFileStream.fail() )
	{
		Log::Event( "Missing configuration file.\n" );

		configurationFileStream.open( "AICritters3.default.ini" );
		if( configurationFileStream.fail() )
		{
			Log::Event( Log::Fatal, "Missing default configuration file.\n" );
		}
	}

	StoredSettings.clear();

	Log::Event( "\nSettings:\n" );

	std::regex FilterSettings = ConfigureFilter( "(.*)" );

	std::string line;
	while( std::getline( configurationFileStream, line ) )
	{
		std::smatch match;
		if( std::regex_search( line, match, FilterSettings ) )
		{
			Log::Event( "%s = %s\n", match[1].str().c_str(), match[2].str().c_str() );
			StoredSettings.insert_or_assign( match[1].str(), match[2].str() );
		}
	}

	Log::Event( "\n" );

	configurationFileStream.close();
}

std::regex CConfiguration::ConfigureFilter( const char* KeyName )
{
	std::stringstream FilterString;
	FilterString << "(?!#)(?!;)";
	FilterString << KeyName;
	FilterString << "=(.*)";

	std::regex FilterSettings( FilterString.str().c_str() );
	return FilterSettings;
}

std::string CConfiguration::GetValue( const char* KeyName )
{
	if( StoredSettings.find( KeyName ) == StoredSettings.end() )
	{
		return "";
	}

	return StoredSettings[KeyName];
}

CConfiguration::CConfiguration()
{
	Initialize( "AICritters3.ini" );
}
