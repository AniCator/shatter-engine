// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <regex>
#include <string>
#include <map>

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
	bool IsValidKey( const char* KeyName );
	bool IsEnabled( const char* KeyName, const bool Default = false );
	const char* GetString( const char* KeyName, const char* Default = "undefined" );
	int GetInteger( const char* KeyName, const int Default = -1 );
	double GetDouble( const char* KeyName, const double Default = -1.0f );
	float GetFloat( const char* KeyName, const float Default = -1.0f );

	void Initialize();
	void SetFile( const StorageCategory::Type& Location, const std::wstring& FilePath );
	void Reload();

	template<typename T>
	void Store( const char* KeyName, const T Value )
	{
		std::stringstream Stream;
		Stream << Value;
		StoredSettings.insert_or_assign( KeyName, Stream.str() );
	};

	void Save();

private:
	static std::regex ConfigureFilter( const char* KeyName );
	const std::string& GetValue( const char* KeyName );

	std::wstring FilePaths[StorageCategory::Maximum];
	std::map<std::string, std::string> StoredSettings;
	bool Initialized = false;
};