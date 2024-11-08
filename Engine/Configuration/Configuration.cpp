// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Configuration.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/Math.h>

#include <filesystem>
#include <fstream>
#include <sstream>

// Version of the configuration file,
// if this is present in the configuration file, but differs from the value defined here, 
// we wipe the configuration file as it is considered out-of-date.
constexpr int ConfigurationVersion = 1;
constexpr const char* VersionKey = "configuration.Version";

time_t GetModificationTime( const std::wstring& FilePath )
{
	struct _stat Buffer;
	const bool Exists = _wstat( FilePath.c_str(), &Buffer ) == 0 || errno == 132;
	if( !Exists )
		return 0;

	return Buffer.st_mtime;
}

bool CConfiguration::IsValidKey( const std::string& KeyName ) const
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

	return Math::Double( GetValue( KeyName ) );
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
	std::mutex Mutex;
	std::unique_lock<std::mutex> Lock( Mutex );

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
				if( !configurationFileStream.fail() )
				{
					Log::Event( "Restoring engine configuration file using \"%S\".\n", DefaultEngineConfigurationFile.c_str() );
				}
			}
		}

		if( !Initialized )
		{
			Log::Event( "\nSettings:\n" );
		}

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
				
				if( !Initialized )
				{
					Log::Event( "%s = %s\n", Key.c_str(), Value.c_str() );
				}

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

		if( !Initialized )
		{
			Log::Event( "\n" );
		}

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

		// Check if the version key has been set.
		if( IsValidKey( VersionKey ) )
		{
			if( GetInteger( VersionKey, ConfigurationVersion ) != ConfigurationVersion )
			{
				// Remove the configuration file.
				remove( std::experimental::filesystem::path( GetFile() ) );

				// Reset state.
				StoredSettings = PreviousSettings;
				Initialized = false;

				// Reload defaults.
				Reload();

				// Assign the new configuration version.
				Store( VersionKey, ConfigurationVersion );
				Save();
			}
		}
		else
		{
			// Assign the configuration version.
			Store( VersionKey, ConfigurationVersion );
			Save();
		}
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

void CConfiguration::Store( const std::string& KeyName, const std::string& Value )
{
	StoredSettings.insert_or_assign( KeyName, Value );
	ExecuteCallback( KeyName, Value );	
}

void CConfiguration::Store( const std::string& Command, const std::function<void(const std::string&)>& Function )
{
	ConsoleCommands.insert_or_assign( Command, Function );
}

void CConfiguration::Execute( const std::string& Command, const std::string& Parameters ) const
{
	const auto Iterator = ConsoleCommands.find( Command );
	if( Iterator == ConsoleCommands.end() )
		return;

	// Execute the function tied to this command.
	Iterator->second( Parameters );
}

bool CConfiguration::IsValidCommand( const std::string& Command ) const
{
	const auto Iterator = ConsoleCommands.find( Command );
	if( Iterator == ConsoleCommands.end() )
		return false;

	return true;
}

void CConfiguration::MarkAsTemporary( const std::string& Key )
{
	ConsoleVariables.insert( Key );
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
			if( ConsoleVariables.find( Setting.first ) != ConsoleVariables.end() )
				continue; // Don't save console variables.

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

void CConfiguration::GetValue( const std::string& Key, bool& Target )
{
	if( !IsValidKey( Key ) )
		return;

	const auto& Value = GetValue( Key );
	Target = Value == "1" || Value == "true";
}

void CConfiguration::GetValue( const std::string& Key, std::string& Target )
{
	if( !IsValidKey( Key ) )
		return;

	Target = GetValue( Key );
}

void CConfiguration::GetValue( const std::string& Key, int& Target )
{
	if( !IsValidKey( Key ) )
		return;

	Target = Math::Integer( GetValue( Key ) );
}

void CConfiguration::GetValue( const std::string& Key, double& Target )
{
	if( !IsValidKey( Key ) )
		return;

	Target = Math::Double( GetValue( Key ) );
}

void CConfiguration::GetValue( const std::string& Key, float& Target )
{
	if( !IsValidKey( Key ) )
		return;

	Target = Math::Float( GetValue( Key ) );
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
