// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <regex>
#include <string>
#include <map>

class CConfiguration
{
public:
	bool IsValidKey( const char* KeyName );
	bool IsEnabled( const char* KeyName );
	const char* GetString( const char* KeyName );
	int GetInteger( const char* KeyName );
	double GetDouble( const char* KeyName );
	float GetFloat( const char* KeyName );

	void Initialize( std::string FilePath );

private:
	std::regex ConfigureFilter( const char* KeyName );
	std::string GetValue( const char* KeyName );

	std::string FileName;
	std::map<std::string, std::string> StoredSettings;

public:
	static CConfiguration& GetInstance()
	{
		static CConfiguration StaticInstance;
		return StaticInstance;
	}
private:
	CConfiguration();

	CConfiguration( CConfiguration const& ) = delete;
	void operator=( CConfiguration const& ) = delete;
};