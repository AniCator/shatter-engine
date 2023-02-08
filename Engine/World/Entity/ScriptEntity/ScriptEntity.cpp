// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ScriptEntity.h"

#include <Engine/Utility/Script/AngelEngine.h>
#include <Engine/Utility/Script/AngelTemplate.h>

static CEntityFactory<ScriptEntity> Factory( "script" );

ScriptEntity::ScriptEntity()
{
	Inputs["Recompile"] = [this]( CEntity* Origin ) 
	{
		LoadScript();
		return true;
	};
}

void ScriptEntity::Construct()
{
	CPointEntity::Construct();

	// Disable ticking.
	NextTickTime = Interval;

	Execute( "Construct" );
	Send( "OnRun" );
}

void ScriptEntity::Destroy()
{
	CPointEntity::Destroy();

	if( Module.Valid() )
	{
		ScriptEngine::Remove( Module.ID );
	}
}

void ScriptEntity::Tick()
{
	CPointEntity::Tick();

	if( TickFunction.empty() )
		return;

	Execute( TickFunction );
	NextTickTime = GetCurrentTime() + Interval;
}

void ScriptEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );

	JSON::Assign( Objects, "script", Script );
}

void ScriptEntity::Reload()
{
	CPointEntity::Reload();

	LoadScript();
}

void ScriptEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );

	Serialize::Import( Data, "s", Script );
	Serialize::Import( Data, "d", Properties );
}

void ScriptEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );

	Serialize::Export( Data, "s", Script );
	Serialize::Export( Data, "d", Properties );
}

void ScriptEntity::Interact( Interactable* Caller )
{
	// Update the interaction entity.
	InteractionEntity = dynamic_cast<CEntity*>( Caller );

	Execute( InteractionFunction );
	Send( "OnInteract" );
}

bool ScriptEntity::CanInteract( Interactable* Caller ) const
{
	return !InteractionFunction.empty();
}

void ScriptEntity::LoadScript()
{
	// Load the script file, if it exists.
	if( !HasScript() )
		return;

	auto File = CFile( Script.c_str() );
	if( !File.Exists() )
	{
		// The script file does not exist.
		Log::Event( Log::Warning, "Could not find script file \"%s\"\n", Script.c_str() );
		return;
	}

	// Load the script file.
	File.Load( true );

	// Keep track of the previous module.
	const auto PreviousModule = Module;

	// Acquire a new UUID.
	Module.Random();

	// Report which script we're compiling and what its assigned ID is.
	Log::Event( "Compiling scripts \"%s\" (%s)\n", File.Location().c_str(), Module.ID );

	// Create a new module.
	if( ScriptEngine::Add( Module.ID, File ) == AngelResult::Success )
	{
		// Remove the previous module.
		if( PreviousModule.Valid() )
		{
			Log::Event( "Removing previous module \"%s\" (%s)\n", File.Location().c_str(), PreviousModule.ID );
			ScriptEngine::Remove( PreviousModule.ID );
		}

		Execute( "Load" );
	}
	else
	{
		// Compilation has failed, keep using the previous module.
		Module = PreviousModule;
	}
}

bool ScriptEntity::Execute( const std::string& Function )
{
	if( !HasModule() )
	{
		// Attempt to load the script, if we have one.
		if( HasScript() )
		{
			LoadScript();
		}

		if( !HasModule() )
			return false; // Module compilation was unsuccesful.
	}

	std::string FunctionDeclaration = "void " + Function + "(Script @self)";
	if( ScriptEngine::Execute( Module.ID, FunctionDeclaration.c_str(), this ) == AngelResult::Success )
		return true;

	return false;
}

bool ScriptEntity::HasFunction( const std::string& Function ) const
{
	if( !HasModule() )
		return false;

	std::string FunctionDeclaration = "void " + Function + "(Script @self)";
	return ScriptEngine::HasFunction( Module.ID, FunctionDeclaration.c_str() );
}

void ScriptEntity::SetTick( const std::string& Function )
{
	TickFunction = Function;
}

void ScriptEntity::SetInterval( const double& Interval )
{
	this->Interval = Interval;
	NextTickTime = GetCurrentTime() + Interval;
}

void ScriptEntity::SetInteract( const std::string& Function )
{
	InteractionFunction = Function;
}

bool ScriptEntity::HasFloat( const std::string& Key )
{
	return Properties.Has( Key, PropertyType::Float );
}

float ScriptEntity::GetFloat( const std::string& Key )
{
	if( const auto Query = Properties.Get( Key, PropertyType::Float ) )
	{
		if( Query )
		{
			return Query().GetFloat();
		}
	}

	return 0.0;
}

void ScriptEntity::SetFloat( const std::string& Key, const float& Value )
{
	Properties.Set( Key, Value );
}

bool ScriptEntity::HasInteger( const std::string& Key )
{
	return Properties.Has( Key, PropertyType::I32 );
}

int32_t ScriptEntity::GetInteger( const std::string& Key )
{
	if( const auto Query = Properties.Get( Key, PropertyType::I32 ) )
	{
		if( Query )
		{
			return Query().GetI32();
		}
	}

	return 0;
}

void ScriptEntity::SetInteger( const std::string& Key, const int32_t& Value )
{
	Properties.Set( Key, Value );
}

bool ScriptEntity::HasString( const std::string& Key )
{
	return Properties.Has( Key, PropertyType::String );
}

std::string ScriptEntity::GetString( const std::string& Key )
{
	if( const auto Query = Properties.Get( Key, PropertyType::String ) )
	{
		if( Query )
		{
			return Query().GetString();
		}
	}

	return std::string();
}

void ScriptEntity::SetString( const std::string& Key, const std::string& Value )
{
	Properties.Set( Key, Value );
}

CEntity* ScriptToEntity( ScriptEntity* Script )
{
	return dynamic_cast<CEntity*>( Script );
}

void RegisterScriptEntity()
{
	ScriptEngine::AddTypeReference( "Script" );
	ScriptEngine::AddFunction( "Entity @ ToEntity( Script @ )", ScriptToEntity );

	// Floating point script properties.
	ScriptEngine::AddTypeMethod( "Script", "bool HasFloat(string &in)", &ScriptEntity::HasFloat );
	ScriptEngine::AddTypeMethod( "Script", "float GetFloat(string &in)", &ScriptEntity::GetFloat );
	ScriptEngine::AddTypeMethod( "Script", "void SetFloat(string &in, float &in)", &ScriptEntity::SetFloat );

	// Integer script properties.
	ScriptEngine::AddTypeMethod( "Script", "bool HasInteger(string &in)", &ScriptEntity::HasInteger );
	ScriptEngine::AddTypeMethod( "Script", "int GetInteger(string &in)", &ScriptEntity::GetInteger );
	ScriptEngine::AddTypeMethod( "Script", "void SetInteger(string &in, int &in)", &ScriptEntity::SetInteger );

	// String script properties.
	ScriptEngine::AddTypeMethod( "Script", "bool HasString(string &in)", &ScriptEntity::HasString );
	ScriptEngine::AddTypeMethod( "Script", "string GetString(string &in)", &ScriptEntity::GetString );
	ScriptEngine::AddTypeMethod( "Script", "void SetString(string &in, string &in)", &ScriptEntity::SetString );

	// Script think functionality.
	ScriptEngine::AddTypeMethod( "Script", "void SetTick(string &in)", &ScriptEntity::SetTick );
	ScriptEngine::AddTypeMethod( "Script", "void SetInterval(double &in)", &ScriptEntity::SetInterval );

	// For user interactions.
	ScriptEngine::AddTypeMethod( "Script", "void SetInteract(string &in)", &ScriptEntity::SetInteract );

	// Public script members.
	ScriptEngine::AddTypeProperty( "Script", "Entity @InteractionEntity", &ScriptEntity::InteractionEntity );
}
