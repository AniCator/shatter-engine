// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Configuration.h"
#include <Profiling/Logging.h>
#include <Profiling/Profiling.h>

#include <fstream>
#include <sstream>

static const char* DefaultEngineConfigurationFile = "ShatterEngine.default.ini";
static const char* EngineConfigurationFile = "ShatterEngine.ini";

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

void CConfiguration::Initialize()
{
	ProfileBare( __FUNCTION__ );

	Log::Event( "Loading configuration.\n" );

	Reload();
}

void CConfiguration::AppendFile( std::string FilePath )
{
	FilePaths.push_back( FilePath );
}

void CConfiguration::Reload()
{
	if( !Initialized )
	{
		// Make sure the engine configuration file is loaded first.
		AppendFile( EngineConfigurationFile );
		Initialized = true;
	}

	StoredSettings.clear();

	bool IsFirstFile = true;
	for( int Index = 0; Index < FilePaths.size(); Index++ )
	{
		std::string& FilePath = FilePaths[Index];
		const char* FilePathCharacterString = FilePath.c_str();
		Log::Event( "Loading \"%s\".\n", FilePathCharacterString );

		std::ifstream configurationFileStream;
		configurationFileStream.open( FilePathCharacterString, std::ios::binary );
		if( configurationFileStream.fail() )
		{
			Log::Event( "Could not find configuration file \"%s\".\n", FilePathCharacterString );

			if( IsFirstFile )
			{
				configurationFileStream.open( DefaultEngineConfigurationFile, std::ios::binary );
				if( configurationFileStream.fail() )
				{
					Log::Event( Log::Fatal, "Cannot restore engine configuration file because \"%s\" is missing.\n", DefaultEngineConfigurationFile );
				}
				else
				{
					std::ofstream defaultConfigurationStream( FilePathCharacterString, std::ios::binary );
					defaultConfigurationStream << configurationFileStream.rdbuf();
					defaultConfigurationStream.close();
				}
			}
		}

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

		IsFirstFile = false;
	}
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
	Initialized = false;
	Initialize();
}
