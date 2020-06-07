// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Configuration.h"
#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <fstream>
#include <sstream>

static const char* DefaultEngineConfigurationFile = "ShatterEngine.default.ini";
static const char* EngineConfigurationFile = "ShatterEngine.ini";

bool CConfiguration::IsValidKey( const char* KeyName )
{
	if( StoredSettings.find( KeyName ) == StoredSettings.end() )
	{
		Log::Event( Log::Warning, "Invalid key \"%s\"\n", KeyName );
		return false;
	}

	return true;
}

bool CConfiguration::IsEnabled( const char* KeyName, const bool Default )
{
	if( !IsValidKey( KeyName ) )
	{
		Store( KeyName, Default );
		Save();
		return Default;
	}

	std::string Value = GetValue( KeyName );

	if( Value.compare( "1" ) == 0 || Value.compare( "true" ) == 0 )
	{
		return true;
	}

	return false;
}

const char* CConfiguration::GetString( const char* KeyName, const char* Default )
{
	if( !IsValidKey( KeyName ) )
	{
		Store( KeyName, Default );
		Save();
		return Default;
	}

	return GetValue( KeyName ).c_str();
}

int CConfiguration::GetInteger( const char* KeyName, const int Default )
{
	if( !IsValidKey( KeyName ) )
	{
		Store( KeyName, Default );
		Save();
		return Default;
	}

	return atoi( GetValue( KeyName ).c_str() );
}

double CConfiguration::GetDouble( const char* KeyName, const double Default )
{
	if( !IsValidKey( KeyName ) )
	{
		Store( KeyName, Default );
		Save();
		return Default;
	}

	return atof( GetValue( KeyName ).c_str() );
}

float CConfiguration::GetFloat( const char* KeyName, const float Default )
{
	return static_cast<float>( GetDouble( KeyName, Default ) );
}

void CConfiguration::Initialize()
{
	ProfileBare( __FUNCTION__ );

	Log::Event( "Loading configuration.\n" );

	Reload();
}

void CConfiguration::AppendFile( const StorageCategory::Type& Location, const std::string& FilePath )
{
	if( Location < StorageCategory::Maximum )
	{
		FilePaths[Location] = FilePath;
	}
	else
	{
		Log::Event( Log::Warning, "Can't append path, invalid configuration storage category.\n" );
	}
}

void CConfiguration::Reload()
{
	if( !Initialized )
	{
		// Make sure the engine configuration file is loaded first.
		AppendFile( StorageCategory::Application, EngineConfigurationFile );
		Initialized = true;
	}

	StoredSettings.clear();

	bool IsFirstFile = true;
	for( const auto& FilePath : FilePaths )
	{
		const char* FilePathCharacterString = FilePath.c_str();
		Log::Event( "Loading \"%s\".\n", FilePathCharacterString );

		std::ifstream configurationFileStream;
		configurationFileStream.open( FilePathCharacterString );
		if( configurationFileStream.fail() )
		{
			Log::Event( "Could not find configuration file \"%s\".\n", FilePathCharacterString );

			if( IsFirstFile )
			{
				configurationFileStream.open( DefaultEngineConfigurationFile );
				if( configurationFileStream.fail() )
				{
					Log::Event( Log::Warning, "Cannot restore engine configuration file because \"%s\" is missing.\n", DefaultEngineConfigurationFile );
				}
				else
				{
					std::ofstream defaultConfigurationStream( FilePathCharacterString );
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
			const bool IgnoreLine = line[0] == ';' || line[0] == '#';
			if( !IgnoreLine && std::regex_search( line, match, FilterSettings ) )
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

void CConfiguration::Save()
{
	std::string SavePath;
	for( const auto& FilePath : FilePaths )
	{
		if( FilePath.length() > 0 )
		{
			SavePath = FilePath;
		}
	}

	if( SavePath.length() > 0 )
	{
		std::ofstream ConfigurationStream;
		ConfigurationStream.open( SavePath.c_str() );

		if( ConfigurationStream.good() )
		{
			Log::Event( "Saving configuration file to \"%s\".\n", SavePath.c_str() );

			for( auto& Setting : StoredSettings )
			{
				ConfigurationStream << Setting.first << "=" << Setting.second << std::endl;
			}
		}

		ConfigurationStream.close();
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

const std::string& CConfiguration::GetValue( const char* KeyName )
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
