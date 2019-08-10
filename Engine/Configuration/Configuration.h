// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <regex>
#include <string>
#include <map>

enum class ECategory : size_t
{
	Miscellaneous = 0,
	Game,
	Audio,
	Graphics,
	Input
};

class CConfiguration
{
public:
	bool IsValidKey( const char* KeyName );
	bool IsEnabled( const char* KeyName, const bool Default = false );
	const char* GetString( const char* KeyName, const char* Default = "undefined" );
	int GetInteger( const char* KeyName, const int Default = -1 );
	double GetDouble( const char* KeyName, const double Default = -1.0f );
	float GetFloat( const char* KeyName, const float Default = -1.0f );

	void Initialize();
	void AppendFile( std::string FilePath );
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
	std::regex ConfigureFilter( const char* KeyName );
	const std::string& GetValue( const char* KeyName );

	std::vector<std::string> FilePaths;
	std::map<std::string, std::string> StoredSettings;
	bool Initialized;

public:
	static CConfiguration& Get()
	{
		static CConfiguration StaticInstance;
		return StaticInstance;
	}
private:
	CConfiguration();

	CConfiguration( CConfiguration const& ) = delete;
	void operator=( CConfiguration const& ) = delete;
};