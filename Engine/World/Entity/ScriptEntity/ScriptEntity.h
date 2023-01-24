// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/PropertyTable.h>

#include <string>

class ScriptEntity : public CPointEntity
{
public:
	ScriptEntity();
	~ScriptEntity() = default;

	void Construct() override;
	void Destroy() override;

	void Tick() override;

	void Load( const JSON::Vector& Objects ) override;
	void Reload() override;

	void Import( CData& Data ) override;
	void Export( CData& Data ) override;

	bool HasScript() const
	{
		return !Script.empty();
	}

	bool HasModule() const
	{
		return Module.Valid();
	}

	void LoadScript();

	// Execute a script function.
	bool Execute( const std::string& Function );
	bool HasFunction( const std::string& Function ) const;

	// Specify which script tick function to use.
	void SetTick( const std::string& Function );

	// Set the interval time between ticks.
	void SetInterval( const double& Interval );

	// Checks if script property is set.
	bool HasFloat( const std::string& Key );

	// Return script property.
	float GetFloat( const std::string& Key );

	// Set script property.
	void SetFloat( const std::string& Key, const float& Value );

	// Checks if script property is set.
	bool HasInteger( const std::string& Key );

	// Return script property.
	int32_t GetInteger( const std::string& Key );

	// Set script property.
	void SetInteger( const std::string& Key, const int32_t& Value );

	// Checks if script property is set.
	bool HasString( const std::string& Key );

	// Return script property.
	std::string GetString( const std::string& Key );

	// Set script property.
	void SetString( const std::string& Key, const std::string& Value );

protected:
	// Path to the script's location.
	std::string Script;

	// The identifier of the module the script is compiled in.
	UniqueIdentifier Module;

	// Name of the function that should be called when ticking.
	std::string TickFunction;

	double Interval = FLT_MAX;

	// Script-controlled properties.
	PropertyTable Properties;
};