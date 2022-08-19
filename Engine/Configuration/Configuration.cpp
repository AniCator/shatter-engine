// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Configuration.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>

#include <fstream>
#include <sstream>

time_t GetModificationTime( const std::wstring& FilePath )
{
	struct _stat Buffer;
	const bool Exists = _wstat( FilePath.c_str(), &Buffer ) == 0 || errno == 132;
	if( !Exists )
		return 0;

	return Buffer.st_mtime;
}

bool CConfiguration::IsValidKey( const std::string& KeyName )
{
	if( StoredSettings.find( KeyName ) == StoredSettings.end() )
	{
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

std::string CConfiguration::GetString( const std::string& KeyName, const std::string& Default )
{
	if( !IsValidKey( KeyName ) )
	{
		Store( KeyName, Default );
		Save();
		return Default;
	}

	return GetValue( KeyName );
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

const std::wstring& CConfiguration::GetFile() const
{
	size_t FileIndex = 0;
	for( size_t Index = 0; Index < StorageCategory::Maximum; Index++ )
	{
		if( FilePaths[Index].length() > 0 )
			FileIndex = Index;
	}

	return FilePaths[FileIndex];
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
	static const std::wstring DefaultEngineConfigurationFile = L"ShatterEngine.default.ini";
	static const std::wstring EngineConfigurationFile = L"ShatterEngine.ini";

	SetFile( StorageCategory::Application, EngineConfigurationFile );

	const auto PreviousSettings = StoredSettings;
	StoredSettings.clear();

	bool FoundMissingSetting = false;

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

		if( !Initialized && !IsFirstFile )
		{
			// Include missing settings when initializing.
			// These are usually global configuration variables that have never been saved before.
			for( const auto& Entry : PreviousSettings )
			{
				const auto& Iterator = StoredSettings.find( Entry.first );
				if( Iterator == StoredSettings.end() )
				{
					StoredSettings.insert_or_assign( Entry.first, Entry.second );
					FoundMissingSetting = true;
				}
			}
		}

		Log::Event( "\n" );

		configurationFileStream.close();

		IsFirstFile = false;
	}

	if( FoundMissingSetting )
	{
		// Make sure the missing entries are saved.
		Save();
	}

	ModificationTime = Math::Max( ModificationTime, GetModificationTime( GetFile() ) );

	if( !Initialized )
	{
		Initialized = true;
	}
}

void CConfiguration::ReloadIfModified()
{
	// Reload the configuration file if it was modified.
	const auto NewModificationTime = GetModificationTime( GetFile() );
	if( Initialized && NewModificationTime > ModificationTime )
	{
		Reload();
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

		std::map<std::string, std::string> SortedSettings;
		SortedSettings.insert( StoredSettings.begin(), StoredSettings.end() );

		for( auto& Setting : SortedSettings )
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

void CConfiguration::Track( const std::string& Key, bool& Target, const bool& Default )
{
	Target = IsEnabled( Key.c_str(), Default );
	SetCallback( Key, [&Target] ( const std::string& Value )
		{
			Target = Value == "1" || Value == "true";
		}
	);
}

void CConfiguration::Track( const std::string& Key, std::string& Target, const std::string& Default )
{
	Target = GetString( Key.c_str(), Default );
	SetCallback( Key, [&Target] ( const std::string& Value )
		{
			Target = Value;
		}
	);
}

void CConfiguration::Track( const std::string& Key, int& Target, const int& Default )
{
	Target = GetInteger( Key.c_str(), Default );
	SetCallback( Key, [&Target] ( const std::string& Value )
		{
			Target = Math::Integer( Value );
		}
	);
}

void CConfiguration::Track( const std::string& Key, double& Target, const double& Default )
{
	Target = GetDouble( Key.c_str(), Default );
	SetCallback( Key, [&Target] ( const std::string& Value )
		{
			Target = Math::Float( Value );
		}
	);
}

void CConfiguration::Track( const std::string& Key, float& Target, const float& Default )
{
	Target = GetFloat( Key.c_str(), Default );
	SetCallback( Key, [&Target] ( const std::string& Value )
		{
			Target = Math::Float( Value );
		}
	);
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

void CConfiguration::ClearCallback( const std::string& Key )
{
	const auto Iterator = Callbacks.find( Key );
	if( Iterator != Callbacks.end() )
	{
		Callbacks.erase( Iterator );
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
