// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Configuration.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/Math.h>

#include <fstream>
#include <sstream>

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

	const auto& Value = GetValue( KeyName );

	if( Value == "1" || Value == "true" )
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

	return Math::Integer( GetValue( KeyName ) );
}

double CConfiguration::GetDouble( const char* KeyName, const double Default )
{
	if( !IsValidKey( KeyName ) )
	{
		Store( KeyName, Default );
		Save();
		return Default;
	}

	return Math::Float( GetValue( KeyName ) );
}

float CConfiguration::GetFloat( const char* KeyName, const float Default )
{
	return static_cast<float>( GetDouble( KeyName, Default ) );
}

void CConfiguration::Initialize()
{
	Log::Event( "Loading configuration.\n" );
	Reload();
}

void CConfiguration::SetFile( const StorageCategory::Type& Location, const std::wstring& FilePath )
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
		Initialized = true;
	}

	static const std::wstring DefaultEngineConfigurationFile = L"ShatterEngine.default.ini";
	static const std::wstring EngineConfigurationFile = L"ShatterEngine.ini";

	SetFile( StorageCategory::Application, EngineConfigurationFile );

	const auto PreviousSettings = StoredSettings;
	StoredSettings.clear();

	bool IsFirstFile = true;
	for( const auto& FilePath : FilePaths )
	{
		const wchar_t* FilePathCharacterString = FilePath.c_str();
		Log::Event( "Loading \"%S\".\n", FilePathCharacterString );

		std::ifstream configurationFileStream;
		configurationFileStream.open( FilePathCharacterString );
		if( configurationFileStream.fail() )
		{
			Log::Event( "Could not find configuration file \"%S\".\n", FilePathCharacterString );

			if( IsFirstFile )
			{
				configurationFileStream.open( DefaultEngineConfigurationFile );
				if( configurationFileStream.fail() )
				{
					Log::Event( Log::Warning, "Cannot restore engine configuration file because \"%S\" is missing.\n", DefaultEngineConfigurationFile.c_str() );
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

		std::string Line;
		while( std::getline( configurationFileStream, Line ) )
		{
			std::smatch Match;
			const bool IgnoreLine = Line[0] == ';' || Line[0] == '#';
			if( !IgnoreLine && std::regex_search( Line, Match, FilterSettings ) )
			{
				const auto& Key = Match[1].str();
				const auto& Value = Match[2].str();
				Log::Event( "%s = %s\n", Key.c_str(), Value.c_str() );
				StoredSettings.insert_or_assign( Key, Value );

				// Check if any callbacks need to be executed.
				if( HasCallback( Key ) )
				{
					const auto Iterator = PreviousSettings.find( Key );
					if( Iterator != PreviousSettings.end() )
					{
						if( Value != Iterator->second )
						{
							ExecuteCallback( Key, Value );
						}
					}
				}
			}
		}

		Log::Event( "\n" );

		configurationFileStream.close();

		IsFirstFile = false;
	}
}

void CConfiguration::Save()
{
	std::wstring SavePath;
	for( const auto& FilePath : FilePaths )
	{
		if( FilePath.length() == 0 )
			continue;
		
		SavePath = FilePath;
	}

	if( SavePath.length() == 0 )
		return;

	std::ofstream ConfigurationStream;
	ConfigurationStream.open( SavePath.c_str() );

	if( ConfigurationStream.good() )
	{
		Log::Event( "Saving configuration file to \"%S\".\n", SavePath.c_str() );

		for( auto& Setting : StoredSettings )
		{
			ConfigurationStream << Setting.first << "=" << Setting.second << std::endl;
		}
	}

	ConfigurationStream.close();
}

void CConfiguration::SetCallback( const std::string& Key, const std::function<void( const std::string& )>& Function )
{
	if( HasCallback( Key ) )
	{
		Log::Event( Log::Warning, "New callback set for \"%s\", previous callback will no longer be active.\n" );
	}

	Callbacks.insert_or_assign( Key, Function );
}

bool CConfiguration::HasCallback( const std::string& Key ) const
{
	const auto Iterator = Callbacks.find( Key );
	return Iterator != Callbacks.end();
}

void CConfiguration::ExecuteCallback( const std::string& Key, const std::string& Value ) const
{
	const auto Iterator = Callbacks.find( Key );
	if( Iterator != Callbacks.end() )
	{
		Iterator->second( Value );
	}
}

std::regex CConfiguration::ConfigureFilter( const char* KeyName )
{
	const std::string FilterString = "(?!#)(?!;)" + std::string( KeyName ) + "=(.*)";
	std::regex FilterSettings( FilterString.c_str() );
	return FilterSettings;
}

const std::string& CConfiguration::GetValue( const std::string& KeyName ) const
{
	const auto Iterator = StoredSettings.find( KeyName );
	if( Iterator == StoredSettings.end() )
	{
		static std::string UnknownEntry( "null" );
		return UnknownEntry;
	}

	return Iterator->second;
}
