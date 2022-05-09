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
	void SetFile( const StorageCategory::Type& Location, const std::wstring& FilePath );
	void Reload();

	template<typename T>
	void Store( const std::string& KeyName, const T Value )
	{
		std::stringstream Stream;
		Stream << Value;
		StoredSettings.insert_or_assign( KeyName, Stream.str() );
		ExecuteCallback( KeyName, Stream.str() );
	}

	void Save();

	/// <summary>
	/// Used to configure a callback that is executed when the key's value changes.
	/// </summary>
	/// <param name="Key">Configuration key that should be monitored.</param>
	/// <param name="Function">To be executed when the given key's value changes.</param>
	void SetCallback( const std::string& Key, const std::function<void( const std::string& )>& Function );
	bool HasCallback( const std::string& Key ) const;
	void ExecuteCallback( const std::string& Key, const std::string& Value ) const;

private:
	static std::regex ConfigureFilter( const char* KeyName );
	const std::string& GetValue( const std::string& KeyName ) const;

	std::wstring FilePaths[StorageCategory::Maximum];
	std::unordered_map<std::string, std::string> StoredSettings;
	std::unordered_map<std::string, std::function<void(std::string)>> Callbacks;
	bool Initialized = false;
};