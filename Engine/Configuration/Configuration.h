// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
#include <regex>
#include <string>
#include <unordered_map>

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
	bool IsValidKey( const std::string& KeyName );
	bool IsEnabled( const char* KeyName, const bool Default = false );
	std::string GetString( const std::string& KeyName, const std::string& Default = "undefined" );
	int GetInteger( const char* KeyName, const int Default = -1 );
	double GetDouble( const char* KeyName, const double Default = -1.0f );
	float GetFloat( const char* KeyName, const float Default = -1.0f );

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

private:
	static std::regex ConfigureFilter( const char* KeyName );
	const std::string& GetValue( const std::string& KeyName ) const;

	std::wstring FilePaths[StorageCategory::Maximum];
	std::unordered_map<std::string, std::string> StoredSettings;
	std::unordered_map<std::string, std::function<void(std::string)>> Callbacks;
	bool Initialized = false;

	time_t ModificationTime = 0;
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
		return Value;
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
		CConfiguration::Get().Store( Callback.Key, Value );
		CConfiguration::Get().Save();
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
