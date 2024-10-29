// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
#include <regex>
#include <string>
#include <unordered_map>
#include <set>

#include <Engine/Utility/Singleton.h>

enum class ECategory : uint8_t
{
	Miscellaneous = 0,
	Game,
	Audio,
	Graphics,
	Input
};

namespace StorageCategory
{
	enum Type
	{
		Application = 0,
		User,
		Maximum
	};
}

class CConfiguration : public Singleton<CConfiguration>
{
public:
	bool IsValidKey( const std::string& KeyName ) const;
	bool IsEnabled( const char* KeyName, const bool Default = false );
	std::string GetString( const std::string& KeyName, const std::string& Default = "undefined" );
	int GetInteger( const char* KeyName, const int Default = -1 );
	double GetDouble( const char* KeyName, const double Default = -1.0f );
	float GetFloat( const char* KeyName, const float Default = -1.0f );

	const std::unordered_map<std::string, std::string>& GetSettings() const
	{
		return StoredSettings;
	}

	void Initialize();
	const std::wstring& GetFile() const;
	void SetFile( const StorageCategory::Type& Location, const std::wstring& FilePath );
	void Reload();

	void ReloadIfModified();

	template<typename T>
	void Store( const std::string& KeyName, const T& Value )
	{
		std::stringstream Stream;
		Stream << Value;
		Store( KeyName, Stream.str() );
	}

	void Store( const std::string& KeyName, const std::string& Value );

	// Create a console command.
	void Store( const std::string& Command, const std::function<void(const std::string&)>& Function );

	// Execute a console command.
	void Execute( const std::string& Command, const std::string& Parameters ) const;

	// Returns true if the console command exists.
	bool IsValidCommand( const std::string& Command ) const;

	const std::unordered_map<std::string, std::function<void( const std::string& )>>& GetCommands() const
	{
		return ConsoleCommands;
	}

	// Stops the specified key from being written to the configuration file. (for console variables)
	void MarkAsTemporary( const std::string& Key );

	void Save();

	/// <summary>
	/// Used to configure a callback that is executed when the key's value changes.
	/// </summary>
	/// <param name="Key">Configuration key that should be monitored.</param>
	/// <param name="Function">To be executed when the given key's value changes.</param>
	void SetCallback( const std::string& Key, const std::function<void( const std::string& )>& Function );

	/// <summary>
	/// Assigns the configured value, if present, and sets a callback for said key.
	/// </summary>
	/// <param name="Key">Configuration key that should be tracked.</param>
	/// <param name="Target">The address the configured value is assigned to.</param>
	/// <param name="Default">The default value that is assigned to the target if the key isn't configured.</param>
	void Track( const std::string& Key, bool& Target, const bool& Default = false );
	void Track( const std::string& Key, std::string& Target, const std::string& Default );
	void Track( const std::string& Key, int& Target, const int& Default = -1 );
	void Track( const std::string& Key, double& Target, const double& Default = 0.0 );
	void Track( const std::string& Key, float& Target, const float& Default = 0.0f );

	void GetValue( const std::string& Key, bool& Target );
	void GetValue( const std::string& Key, std::string& Target );
	void GetValue( const std::string& Key, int& Target );
	void GetValue( const std::string& Key, double& Target );
	void GetValue( const std::string& Key, float& Target );

	bool HasCallback( const std::string& Key ) const;
	void ExecuteCallback( const std::string& Key, const std::string& Value ) const;
	void ClearCallback( const std::string& Key );

	template<typename T>
	struct CallbackTracker
	{
		CallbackTracker() = default;
		CallbackTracker( const std::string& Key, T& Target, const T& Default )
		{
			this->Key = Key;
			if( !Key.empty() )
				Get().Track( Key, Target, Default );
		}

		~CallbackTracker()
		{
			if( !Key.empty() )
				Get().ClearCallback( Key );
		}

		CallbackTracker( const CallbackTracker& ) = delete;
		CallbackTracker& operator=( const CallbackTracker& ) = delete;

		CallbackTracker( CallbackTracker&& RHS ) noexcept
		{
			std::swap( Key, RHS.Key );
		}

		CallbackTracker& operator=( CallbackTracker&& RHS ) noexcept
		{
			Key = std::move( RHS.Key );
			return *this;
		}

		std::string Key;
	};

	// Only use this if you want direct access.
	const std::string& GetValue( const std::string& KeyName ) const;

private:
	static std::regex ConfigureFilter( const char* KeyName );

	std::wstring FilePaths[StorageCategory::Maximum];
	std::unordered_map<std::string, std::string> StoredSettings;
	std::unordered_map<std::string, std::function<void(const std::string&)>> Callbacks;
	bool Initialized = false;

	time_t ModificationTime = 0;

	// Variables that should not be saved to disk.
	std::set<std::string> ConsoleVariables;

	// Commands that can be executed using the console.
	std::unordered_map<std::string, std::function<void(const std::string&)>> ConsoleCommands;
};

template<typename T>
struct ConfigurationVariable
{
	ConfigurationVariable() = delete;
	ConfigurationVariable( const std::string& Name, const T& Default )
	{
		Value = Default;
		Callback = { Name, Value, Default };
	}

	T Get() const
	{
		return Value;
	}

	void Set( const T& Value )
	{
		CConfiguration::Get().Store( Callback.Key, Value );
	}

	explicit operator bool() const
	{
		return !!Value;
	}

protected:
	CConfiguration::CallbackTracker<T> Callback;
	T Value;
};

// Similar to ConfigurationVariable, but only valid for the current session.
template<typename T>
struct ConsoleVariable
{
	ConsoleVariable() = delete;
	ConsoleVariable( const std::string& Name, const T& Default )
	{
		Value = Default;
		Callback = { Name, Value, Default };

		// Overwrite whatever entry is in the configuration file from the previous sessions.
		CConfiguration::Get().MarkAsTemporary( Callback.Key );
		CConfiguration::Get().Store( Callback.Key, Value );
	}

	T Get() const
	{
		return Value;
	}

	void Set( const T& Value )
	{
		CConfiguration::Get().Store( Callback.Key, Value );
	}

	explicit operator bool() const
	{
		return Value;
	}

protected:
	CConfiguration::CallbackTracker<T> Callback;
	T Value;
};

// Similar to ConfigurationVariable, but only valid for the current session.
template<typename T>
using ConVar = ConsoleVariable<T>;

// Read-only ConfigurationVariable, less performant (performs a lookup every time).
template<typename T>
struct ConfigurationReference
{
	ConfigurationReference() = delete;
	ConfigurationReference( const std::string& Name )
	{
		Key = Name;
	}

	T Get() const
	{
		T Value = {};
		CConfiguration::Get().GetValue( Key, Value );
		return Value;
	}

	explicit operator bool() const
	{
		return !!Get();
	}

protected:
	std::string Key;
};

template<typename T>
using ConRef = ConfigurationReference<T>;


struct ConsoleCommand
{
	ConsoleCommand() = delete;
	ConsoleCommand( const std::string& Name, const std::function<void( const std::string& )> Function )
	{
		CConfiguration::Get().Store( Name, Function );
	}
};

using ConCommand = ConsoleCommand;
