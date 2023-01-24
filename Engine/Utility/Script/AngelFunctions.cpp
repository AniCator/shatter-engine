// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AngelFunctions.h"
#include "AngelEngine.h"
#include "AngelTemplate.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Math.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/ScriptEntity/ScriptEntity.h>
#include <Engine/World/World.h>

#include <Game/Game.h>

void RegisterVectorType()
{
	ScriptEngine::AddTypePOD<Vector3D>( "Vector3D" );
	ScriptEngine::AddTypeProperty( "Vector3D", "float X", &Vector3D::X );
	ScriptEngine::AddTypeProperty( "Vector3D", "float Y", &Vector3D::Y );
	ScriptEngine::AddTypeProperty( "Vector3D", "float Z", &Vector3D::Z );

	// Operators
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opAdd(float &in) const", static_cast<Vector3D( Vector3D::* )( const float& )const>( &Vector3D::operator+ ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opAdd(Vector3D &in) const", static_cast<Vector3D( Vector3D::* )( const Vector3D& )const>( &Vector3D::operator+ ) );

	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opSub(float &in) const", static_cast<Vector3D( Vector3D::* )( const float& )const>( &Vector3D::operator- ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opSub(Vector3D &in) const", static_cast<Vector3D( Vector3D::* )( const Vector3D& )const>( &Vector3D::operator- ) );

	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opMul(float &in) const", static_cast<Vector3D( Vector3D::* )( const float& )const>( &Vector3D::operator* ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opMul(Vector3D &in) const", static_cast<Vector3D( Vector3D::* )( const Vector3D& )const>( &Vector3D::operator* ) );

	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opDiv(float &in) const", static_cast<Vector3D( Vector3D::* )( const float& )const>( &Vector3D::operator/ ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opDiv(Vector3D &in) const", static_cast<Vector3D( Vector3D::* )( const Vector3D& )const>( &Vector3D::operator/ ) );

	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opAddAssign(float &in)", static_cast<Vector3D( Vector3D::* )( const float& )>( &Vector3D::operator+= ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opAddAssign(Vector3D &in)", static_cast<Vector3D( Vector3D::* )( const Vector3D& )>( &Vector3D::operator+= ) );

	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opSubAssign(float &in)", static_cast<Vector3D( Vector3D::* )( const float& )>( &Vector3D::operator-= ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opSubAssign(Vector3D &in)", static_cast<Vector3D( Vector3D::* )( const Vector3D& )>( &Vector3D::operator-= ) );

	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opMulAssign(float &in)", static_cast<Vector3D( Vector3D::* )( const float& )>( &Vector3D::operator*= ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opMulAssign(Vector3D &in)", static_cast<Vector3D( Vector3D::* )( const Vector3D& )>( &Vector3D::operator*= ) );

	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opDivAssign(float &in)", static_cast<Vector3D( Vector3D::* )( const float& )>( &Vector3D::operator/= ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D opDivAssign(Vector3D &in)", static_cast<Vector3D( Vector3D::* )( const Vector3D& )>( &Vector3D::operator/= ) );

	// Functions
	ScriptEngine::AddTypeMethod( "Vector3D", "float Length() const", static_cast<float( Vector3D::* )( void )const>( &Vector3D::Length ) );
	ScriptEngine::AddTypeMethod( "Vector3D", "float LengthSquared() const", &Vector3D::LengthSquared );
	ScriptEngine::AddTypeMethod( "Vector3D", "float Dot(Vector3D &in) const", &Vector3D::Dot );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D Cross(Vector3D &in) const", &Vector3D::Cross );
	ScriptEngine::AddTypeMethod( "Vector3D", "Vector3D Normalized() const", &Vector3D::Normalized );
	ScriptEngine::AddTypeMethod( "Vector3D", "float Normalize()", &Vector3D::Normalize );
	ScriptEngine::AddTypeMethod( "Vector3D", "float Distance(Vector3D &in) const", &Vector3D::Distance );
	ScriptEngine::AddTypeMethod( "Vector3D", "float DistanceSquared(Vector3D &in) const", &Vector3D::DistanceSquared );
	ScriptEngine::AddTypeMethod( "Vector3D", "bool IsNaN() const", &Vector3D::IsNaN );
	ScriptEngine::AddTypeMethod( "Vector3D", "bool IsInfinite() const", &Vector3D::IsInfinite );
	ScriptEngine::AddTypeMethod( "Vector3D", "bool IsValid() const", &Vector3D::IsValid );
}

void LoadSound( const std::string& Name, const std::string& Location )
{
	auto& Assets = CAssets::Get();
	Assets.CreateNamedSound( Name.c_str(), Location.c_str() );
}

int32_t PlaySoundByName( const std::string& Name )
{
	auto& Assets = CAssets::Get();
	if( CSound* Sound = Assets.Sounds.Find( Name ) )
	{
		return Sound->Start();
	}

	return -1;
}

void StopSoundByName( const std::string& Name )
{
	auto& Assets = CAssets::Get();
	if( CSound* Sound = Assets.Sounds.Find( Name ) )
	{
		Sound->Stop();
	}
}

void StopSoundByHandle( const int32_t& Handle )
{
	if( Handle < 0 )
		return;

	SoundHandle SoundHandle;
	SoundHandle.Handle = Handle;
	SoLoudSound::Stop( SoundHandle );
}

void RegisterSound()
{
	ScriptEngine::AddFunction( "void LoadSound( const string &in, const string& in )", LoadSound );
	ScriptEngine::AddFunction( "int PlaySound( const string &in )", PlaySoundByName );
	ScriptEngine::AddFunction( "void StopSound( const int &in )", StopSoundByHandle );
	ScriptEngine::AddFunction( "void StopSound( const string &in )", StopSoundByName );
}

void LoadStream( const std::string& Name, const std::string& Location )
{
	auto& Assets = CAssets::Get();
	Assets.CreateNamedStream( Name.c_str(), Location.c_str() );
}

int32_t PlayStreamByName( const std::string& Name )
{
	auto& Assets = CAssets::Get();
	if( CSound* Sound = Assets.Sounds.Find( Name ) )
	{
		return Sound->Start();
	}

	return -1;
}

void StopStreamByName( const std::string& Name )
{
	auto& Assets = CAssets::Get();
	if( CSound* Stream = Assets.Sounds.Find( Name ) )
	{
		Stream->Stop();
	}
}

void StopStreamByHandle( const int32_t& Handle )
{
	if( Handle < 0 )
		return;

	StreamHandle StreamHandle;
	StreamHandle.Handle = Handle;
	SoLoudSound::Stop( StreamHandle );
}

void RegisterStream()
{
	ScriptEngine::AddFunction( "void LoadStream( const string &in, const string& in )", LoadStream );
	ScriptEngine::AddFunction( "int PlayStream( const string &in )", PlayStreamByName );
	ScriptEngine::AddFunction( "void StopStream( const int &in )", StopStreamByHandle );
	ScriptEngine::AddFunction( "void StopStream( const string &in )", StopStreamByName );
}

void LoadSequence( const std::string& Name, const std::string& Location )
{
	auto& Assets = CAssets::Get();
	Assets.CreateNamedSequence( Name.c_str(), Location.c_str() );
}

void PlaySequence( const std::string& Name )
{
	auto& Assets = CAssets::Get();
	if( CSequence* Sequence = Assets.Sequences.Find( Name ) )
	{
		Sequence->Play();
	}
}

void StopSequence( const std::string& Name )
{
	auto& Assets = CAssets::Get();
	if( CSequence* Sequence = Assets.Sequences.Find( Name ) )
	{
		Sequence->Stop();
	}
}

void RegisterSequence()
{
	ScriptEngine::AddFunction( "void LoadSequence( const string &in, const string& in )", LoadSequence );
	ScriptEngine::AddFunction( "void PlaySequence( const string &in )", PlaySequence );
	ScriptEngine::AddFunction( "void StopSequence( const string &in )", StopSequence );
}

CEntity* GetEntityByName( const std::string& Name )
{
	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return nullptr;

	return World->Find( Name );
}

std::string GetEntityName( CEntity* Entity )
{
	if( !Entity )
		return {};

	return Entity->Name.String();
}

CEntity* SpawnEntity( const std::string& Name, const std::string& Class )
{
	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return nullptr;

	auto* Level = World->GetActiveLevel();
	if( !Level )
		return nullptr;

	return Level->Spawn( Class, Name );
}

void SendEntity( CEntity* Entity, const std::string& Input )
{
	if( !Entity )
	{
		Log::Event( Log::Warning, "SendEntity: Receiving entity invalid.\n" );
		return;
	}

	Entity->Receive( Input.c_str() );
}

void AddEntityInput( CEntity* Entity, const std::string& Function )
{
	if( !Entity )
	{
		Log::Event( Log::Warning, "AddEntityInput: Input entity invalid.\n" );
		return;
	}

	ScriptEntity* Script = dynamic_cast<ScriptEntity*>( Entity );
	if( !Script )
	{
		Log::Event( Log::Warning, "AddEntityInput: Can't assign script inputs to non-script entities.\n" );
		return;
	}

	if( !Script->HasFunction( Function ) )
	{
		Log::Event( Log::Warning,
			"AddEntityInput: Script entity \"%s\" does not have script function \"void %s()\".\n",
			Script->Name.String().c_str(), Function.c_str() );
		return;
	}

	Script->Inputs[Function] = [Script, Function]( CEntity* Origin )
	{
		return Script->Execute( Function );
	};
}

void AddEntityOutput( CEntity* Object, const std::string& Output, CEntity* Target, const std::string& Input )
{
	if( !Object )
	{
		Log::Event( Log::Warning, "AddEntityOutput: Output entity invalid.\n" );
		return;
	}

	if( !Target )
	{
		Log::Event( Log::Warning, "AddEntityOutput: Target entity invalid.\n" );
		return;
	}

	FMessage Message;
	Message.TargetID = Target->GetEntityID();
	Message.TargetName = Target->Name.String();
	Message.Inputs.emplace_back( Input );

	Object->Outputs[Output].emplace_back( Message );
}

void RegisterEntity()
{
	ScriptEngine::AddTypeReference( "Entity" );
	ScriptEngine::AddFunction( "Entity @ GetEntityByName( string &in )", GetEntityByName );
	ScriptEngine::AddFunction( "string GetEntityName( Entity @ )", GetEntityName );
	ScriptEngine::AddFunction( "Entity @ Spawn( string &in, string &in )", SpawnEntity );
	ScriptEngine::AddFunction( "void Send( Entity @, string &in )", SendEntity );
	ScriptEngine::AddFunction( "void AddInput( Entity @, string &in )", AddEntityInput );
	ScriptEngine::AddFunction( "void AddOutput( Entity @, string &in, Entity @, string &in )", AddEntityOutput );
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
}

double GlobalCurrentTime()
{
	return GameLayersInstance->GetCurrentTime();
}

double GlobalRealTime()
{
	return GameLayersInstance->GetRealTime();
}

void RegisterScriptFunctions()
{
	RegisterVectorType();
	RegisterSound();
	RegisterStream();
	RegisterSequence();
	RegisterEntity();
	RegisterScriptEntity();

	// Time functions.
	ScriptEngine::AddFunction( "double GetCurrentTime()", GlobalCurrentTime );
	ScriptEngine::AddFunction( "double GetRealTime()", GlobalRealTime );

	// Math library
	ScriptEngine::AddFunction( "float Random()", Math::Random );

	ScriptEngine::AddFunction( "float ToRadians( float &in )", static_cast<float( * )( const float& )>( Math::ToRadians ) );
	ScriptEngine::AddFunction( "float ToDegrees( float &in )", static_cast<float( * )( const float& )>( Math::ToDegrees ) );
}

void RegisterShatterEngine()
{
	RegisterScriptFunctions();
}