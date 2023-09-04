// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AngelFunctions.h"
#include "AngelEngine.h"
#include "AngelTemplate.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Audio/SoundInstance.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Utility/Math.h>
#include <Engine/World/Entity/Entity.h>
#include <Engine/World/Entity/ScriptEntity/ScriptEntity.h>
#include <Engine/World/Entity/MeshEntity/MeshEntity.h>
#include <Engine/World/World.h>

#include <Game/Game.h>

void RegisterScriptEntity();

void RegisterVector3Type()
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

void RegisterVector4Type()
{
	ScriptEngine::AddTypePOD<Vector4D>( "Vector4D" );
	ScriptEngine::AddTypeProperty( "Vector4D", "float X", &Vector4D::X );
	ScriptEngine::AddTypeProperty( "Vector4D", "float Y", &Vector4D::Y );
	ScriptEngine::AddTypeProperty( "Vector4D", "float Z", &Vector4D::Z );
	ScriptEngine::AddTypeProperty( "Vector4D", "float W", &Vector4D::W );

	// Operators
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opAdd(float &in) const", static_cast<Vector4D( Vector4D::* )( const float& )const>( &Vector4D::operator+ ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opAdd(Vector4D &in) const", static_cast<Vector4D( Vector4D::* )( const Vector4D& )const>( &Vector4D::operator+ ) );

	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opSub(float &in) const", static_cast<Vector4D( Vector4D::* )( const float& )const>( &Vector4D::operator- ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opSub(Vector4D &in) const", static_cast<Vector4D( Vector4D::* )( const Vector4D& )const>( &Vector4D::operator- ) );

	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opMul(float &in) const", static_cast<Vector4D( Vector4D::* )( const float& )const>( &Vector4D::operator* ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opMul(Vector4D &in) const", static_cast<Vector4D( Vector4D::* )( const Vector4D& )const>( &Vector4D::operator* ) );

	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opDiv(float &in) const", static_cast<Vector4D( Vector4D::* )( const float& )const>( &Vector4D::operator/ ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opDiv(Vector4D &in) const", static_cast<Vector4D( Vector4D::* )( const Vector4D& )const>( &Vector4D::operator/ ) );

	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opAddAssign(float &in)", static_cast<Vector4D( Vector4D::* )( const float& )>( &Vector4D::operator+= ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opAddAssign(Vector4D &in)", static_cast<Vector4D( Vector4D::* )( const Vector4D& )>( &Vector4D::operator+= ) );

	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opSubAssign(float &in)", static_cast<Vector4D( Vector4D::* )( const float& )>( &Vector4D::operator-= ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opSubAssign(Vector4D &in)", static_cast<Vector4D( Vector4D::* )( const Vector4D& )>( &Vector4D::operator-= ) );

	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opMulAssign(float &in)", static_cast<Vector4D( Vector4D::* )( const float& )>( &Vector4D::operator*= ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opMulAssign(Vector4D &in)", static_cast<Vector4D( Vector4D::* )( const Vector4D& )>( &Vector4D::operator*= ) );

	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opDivAssign(float &in)", static_cast<Vector4D( Vector4D::* )( const float& )>( &Vector4D::operator/= ) );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D opDivAssign(Vector4D &in)", static_cast<Vector4D( Vector4D::* )( const Vector4D& )>( &Vector4D::operator/= ) );

	// Functions
	ScriptEngine::AddTypeMethod( "Vector4D", "float Length() const", static_cast<float( Vector4D::* )( void )const>( &Vector4D::Length ) );
	// ScriptEngine::AddTypeMethod( "Vector4D", "float LengthSquared() const", &Vector4D::LengthSquared );
	ScriptEngine::AddTypeMethod( "Vector4D", "float Dot(Vector4D &in) const", &Vector4D::Dot );
	// ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D Cross(Vector4D &in) const", &Vector4D::Cross );
	ScriptEngine::AddTypeMethod( "Vector4D", "Vector4D Normalized() const", &Vector4D::Normalized );
	// ScriptEngine::AddTypeMethod( "Vector4D", "float Normalize()", &Vector4D::Normalize );
	ScriptEngine::AddTypeMethod( "Vector4D", "float Distance(Vector4D &in) const", &Vector4D::Distance );
	// ScriptEngine::AddTypeMethod( "Vector4D", "float DistanceSquared(Vector4D &in) const", &Vector4D::DistanceSquared );
	// ScriptEngine::AddTypeMethod( "Vector4D", "bool IsNaN() const", &Vector4D::IsNaN );
	// ScriptEngine::AddTypeMethod( "Vector4D", "bool IsInfinite() const", &Vector4D::IsInfinite );
	// ScriptEngine::AddTypeMethod( "Vector4D", "bool IsValid() const", &Vector4D::IsValid );
}

template<typename T>
void RegisterSoundType( const char* Type )
{
	ScriptEngine::AddTypeReference( Type );
	ScriptEngine::AddTypeMethod( Type, "int Start( const Spatial &in )", &T::Start );
	ScriptEngine::AddTypeMethod( Type, "void Stop( const float &in )", &T::Stop );
	ScriptEngine::AddTypeMethod( Type, "void Loop( const bool &in )", &T::Loop );
	ScriptEngine::AddTypeMethod( Type, "void Rate( const bool &in )", &T::Rate );
	ScriptEngine::AddTypeMethod( Type, "double Time() const", &T::Time );
	ScriptEngine::AddTypeMethod( Type, "double Length() const", &T::Length );
	ScriptEngine::AddTypeMethod( Type, "void Offset( const double &in )", &T::Offset );
	ScriptEngine::AddTypeMethod( Type, "double Playing() const", &T::Playing );
	ScriptEngine::AddTypeMethod( Type, "void Volume( const float &in )", &T::Volume );
	ScriptEngine::AddTypeMethod( Type, "void Fade( const float &in, const float &in )", &T::Fade );
	ScriptEngine::AddTypeMethod( Type, "void Update( const Vector3D &in, const Vector3D &in )", &T::Update );
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

void RegisterSpatial()
{
	ScriptEngine::AddTypePOD<Spatial>( "Spatial" );
	ScriptEngine::AddTypeProperty( "Spatial", "bool Is3D", &Spatial::Is3D );
	ScriptEngine::AddTypeProperty( "Spatial", "Vector3D Position", &Spatial::Position );
	ScriptEngine::AddTypeProperty( "Spatial", "Vector3D Velocity", &Spatial::Velocity );
	ScriptEngine::AddTypeProperty( "Spatial", "float MinimumDistance", &Spatial::MinimumDistance );
	ScriptEngine::AddTypeProperty( "Spatial", "float MaximumDistance", &Spatial::MaximumDistance );
	ScriptEngine::AddTypeProperty( "Spatial", "bool DelayByDistance", &Spatial::DelayByDistance );
	ScriptEngine::AddTypeProperty( "Spatial", "int Attenuation", &Spatial::Attenuation );
	ScriptEngine::AddTypeProperty( "Spatial", "float Rolloff", &Spatial::Rolloff );
	ScriptEngine::AddTypeProperty( "Spatial", "float Doppler", &Spatial::Doppler );
	ScriptEngine::AddTypeProperty( "Spatial", "int Bus", &Spatial::Bus );
	ScriptEngine::AddTypeProperty( "Spatial", "bool StartPaused", &Spatial::StartPaused );
	ScriptEngine::AddTypeProperty( "Spatial", "float FadeIn", &Spatial::FadeIn );
	ScriptEngine::AddTypeProperty( "Spatial", "float Volume", &Spatial::Volume );
	ScriptEngine::AddTypeProperty( "Spatial", "float Rate", &Spatial::Rate );
}

void RegisterSound()
{
	RegisterSpatial();

	ScriptEngine::AddFunction( "void LoadSound( const string &in, const string& in )", LoadSound );
	ScriptEngine::AddFunction( "int PlaySound( const string &in )", PlaySoundByName );
	ScriptEngine::AddFunction( "void StopSound( const int &in )", StopSoundByHandle );
	ScriptEngine::AddFunction( "void StopSound( const string &in )", StopSoundByName );

	RegisterSoundType<CSound>( "Sound" );
	RegisterSoundType<SoundInstance>( "SoundInstance" );

	RegisterStream();
}

CSequence* LoadSequence( const std::string& Name, const std::string& Location )
{
	auto& Assets = CAssets::Get();
	return Assets.CreateNamedSequence( Name.c_str(), Location.c_str() );
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
	ScriptEngine::AddTypeReference( "Sequence" );

	ScriptEngine::AddFunction( "Sequence @ LoadSequence( const string &in, const string& in )", LoadSequence );
	ScriptEngine::AddFunction( "void PlaySequence( const string &in )", PlaySequence );
	ScriptEngine::AddFunction( "void StopSequence( const string &in )", StopSequence );

	ScriptEngine::AddTypeMethod( "Sequence", "string Location() const", &CSequence::Location );

	ScriptEngine::AddTypeMethod( "Sequence", "void Play()", &CSequence::Play );
	ScriptEngine::AddTypeMethod( "Sequence", "void Pause()", &CSequence::Pause );
	ScriptEngine::AddTypeMethod( "Sequence", "void Stop()", &CSequence::Stop );

	ScriptEngine::AddTypeMethod( "Sequence", "bool Playing() const", &CSequence::Playing );
	ScriptEngine::AddTypeMethod( "Sequence", "bool Stopped() const", &CSequence::Stopped );

	ScriptEngine::AddTypeMethod( "Sequence", "void Step()", &CSequence::Step );
	ScriptEngine::AddTypeMethod( "Sequence", "void GoTo( uint64 )", &CSequence::GoTo );

	ScriptEngine::AddTypeMethod( "Sequence", "uint64 CurrentMarker() const", &CSequence::CurrentMarker );
	ScriptEngine::AddTypeMethod( "Sequence", "uint64 Size() const", &CSequence::Size );

	ScriptEngine::AddTypeMethod( "Sequence", "double Time() const", &CSequence::Time );
	ScriptEngine::AddTypeMethod( "Sequence", "double Length() const", &CSequence::Length );
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

CEntity* CopyEntity( CEntity* Entity )
{
	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return nullptr;

	auto* Level = World->GetActiveLevel();
	if( !Level )
		return nullptr;

	UniqueIdentifier Random;
	Random.Random();
	auto* Copy = Level->Spawn( Entity->ClassName, Random.ID );

	// Transfer data.
	CData Data;
	Entity->Export( Data );
	Copy->Import( Data );
	Copy->Reload();
	Copy->Construct();

	return Copy;
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

void SetEntityPosition( CEntity* Object, const Vector3D& Position )
{
	if( !Object )
	{
		Log::Event( Log::Warning, "SetEntityPosition: Entity invalid.\n" );
		return;
	}

	auto* Point = dynamic_cast<CPointEntity*>( Object );
	if( !Point )
	{
		Log::Event( Log::Warning, "SetEntityPosition: Entity is not a point entity.\n" );
		return;
	}

	// Override the position.
	auto Transform = Point->GetTransform();
	Transform.SetPosition( Position );
	Point->SetTransform( Transform );
}

void SetEntitySize( CEntity* Object, const Vector3D& Size )
{
	if( !Object )
	{
		Log::Event( Log::Warning, "SetEntitySize: Entity invalid.\n" );
		return;
	}

	auto* Point = dynamic_cast<CPointEntity*>( Object );
	if( !Point )
	{
		Log::Event( Log::Warning, "SetEntitySize: Entity is not a point entity.\n" );
		return;
	}

	// Override the size.
	auto Transform = Point->GetTransform();
	Transform.SetSize( Size );
	Point->SetTransform( Transform );
}

void SetEntityVelocity( CEntity* Object, const Vector3D& Velocity )
{
	if( !Object )
	{
		Log::Event( Log::Warning, "SetEntityVelocity: Entity invalid.\n" );
		return;
	}

	auto* Mesh = dynamic_cast<CMeshEntity*>( Object );
	if( !Mesh )
	{
		Log::Event( Log::Warning, "SetEntityVelocity: Entity is not a mesh entity.\n" );
		return;
	}

	auto* Body = Mesh->GetBody();
	if( !Body )
	{
		Log::Event( Log::Warning, "SetEntityVelocity: Entity has no physics body.\n" );
		return;
	}

	// Set the velocity.
	Body->Velocity = Velocity;
}

void SetEntityColor( CEntity* Object, const Vector4D& Color )
{
	if( !Object )
	{
		Log::Event( Log::Warning, "SetEntityColor: Entity invalid.\n" );
		return;
	}

	auto* Mesh = dynamic_cast<CMeshEntity*>( Object );
	if( !Mesh )
	{
		Log::Event( Log::Warning, "SetEntityColor: Entity is not a mesh entity.\n" );
		return;
	}

	// Override the previously set object color.
	Mesh->Color = Color;
}

void RegisterEntity()
{
	ScriptEngine::AddTypeReference( "Entity" );
	ScriptEngine::AddFunction( "Entity @ GetEntityByName( string &in )", GetEntityByName );
	ScriptEngine::AddFunction( "string GetEntityName( Entity @ )", GetEntityName );
	ScriptEngine::AddFunction( "Entity @ Spawn( string &in, string &in )", SpawnEntity );
	ScriptEngine::AddFunction( "Entity @ Copy( Entity @ )", CopyEntity );
	ScriptEngine::AddFunction( "void Send( Entity @, string &in )", SendEntity );
	ScriptEngine::AddFunction( "void AddInput( Entity @, string &in )", AddEntityInput );
	ScriptEngine::AddFunction( "void AddOutput( Entity @, string &in, Entity @, string &in )", AddEntityOutput );

	ScriptEngine::AddFunction( "void SetPosition( Entity @, Vector3D &in )", SetEntityPosition );
	ScriptEngine::AddFunction( "void SetSize( Entity @, Vector3D &in )", SetEntitySize );
	ScriptEngine::AddFunction( "void SetVelocity( Entity @, Vector3D &in )", SetEntityVelocity );
	ScriptEngine::AddFunction( "void SetColor( Entity @, Vector4D &in )", SetEntityColor );

	// Entity methods.
	ScriptEngine::AddTypeMethod( "Entity", "void SetParent(Entity &in)", &CEntity::SetParent );
	ScriptEngine::AddTypeMethod( "Entity", "Entity @ GetParent() const", &CEntity::GetParent );

	ScriptEngine::AddTypeMethod( "Entity", "void Send(string &in, Entity @)", &CEntity::Send );
	ScriptEngine::AddTypeMethod( "Entity", "void Receive(string &in, Entity @)", &CEntity::Receive );
	ScriptEngine::AddTypeMethod( "Entity", "void Tag(string &in)", &CEntity::Tag );
	ScriptEngine::AddTypeMethod( "Entity", "void Untag(string &in)", &CEntity::Untag );
	ScriptEngine::AddTypeMethod( "Entity", "bool HasTag(string &in) const", &CEntity::HasTag );

	ScriptEngine::AddTypeMethod( "Entity", "bool IsDebugEnabled() const", &CEntity::IsDebugEnabled );
	ScriptEngine::AddTypeMethod( "Entity", "void EnableDebug( const bool )", &CEntity::EnableDebug );
}

double GlobalCurrentTime()
{
	return GameLayersInstance->GetCurrentTime();
}

double GlobalRealTime()
{
	return GameLayersInstance->GetRealTime();
}

void RegisterTime()
{
	ScriptEngine::AddFunction( "double GetCurrentTime()", GlobalCurrentTime );
	ScriptEngine::AddFunction( "double GetRealTime()", GlobalRealTime );
}

void RegisterMath()
{
	ScriptEngine::AddFunction( "float Random()", Math::Random );
	ScriptEngine::AddFunction( "float Saturate( float& in )", static_cast<float( * )( const float& )>( Math::Saturate ) );
	ScriptEngine::AddFunction( "Vector3D HSVToRGB( Vector3D &in )", Math::HSVToRGB );
	ScriptEngine::AddFunction( "float pow( float, float )", powf );
	ScriptEngine::AddFunction( "float sin( float )", sinf );
	ScriptEngine::AddFunction( "float cos( float )", cosf );
	ScriptEngine::AddFunction( "float tan( float )", tanf );

	ScriptEngine::AddFunction( "float ToRadians( float &in )", static_cast<float( * )( const float& )>( Math::ToRadians ) );
	ScriptEngine::AddFunction( "float ToDegrees( float &in )", static_cast<float( * )( const float& )>( Math::ToDegrees ) );
}

static void ConstructProperty( Property* This )
{
	// Allocate a new property using placement-new.
	new ( This ) Property();
}

template<typename T>
static void CopyConstructProperty( Property* This, const T& Other )
{
	new ( This ) Property( Other );
}

static void DestructProperty( Property* This )
{
	This->~Property();
}

void RegisterProperty()
{
	ScriptEngine::AddEnum( "PropertyType", {
		{ "Unknown", static_cast<int>( PropertyType::Unknown ) },
		{ "String", static_cast<int>( PropertyType::String ) },
		{ "Float", static_cast<int>( PropertyType::Float ) },
		{ "Vector3D", static_cast<int>( PropertyType::Vector3D ) },
		{ "U64", static_cast<int>( PropertyType::U64 ) },
		{ "U32", static_cast<int>( PropertyType::U32 ) },
		{ "U16", static_cast<int>( PropertyType::U16 ) },
		{ "U8", static_cast<int>( PropertyType::U8 ) },
		{ "I64", static_cast<int>( PropertyType::I64 ) },
		{ "I32", static_cast<int>( PropertyType::I32 ) },
		{ "I16", static_cast<int>( PropertyType::I16 ) },
		{ "I8", static_cast<int>( PropertyType::I8 ) },
		{ "Boolean", static_cast<int>( PropertyType::Boolean ) },
		{ "Pointer", static_cast<int>( PropertyType::Pointer ) },
		{ "Data", static_cast<int>( PropertyType::Data ) }
		}
	);

	ScriptEngine::AddTypeValue<Property>( "Property" );
	ScriptEngine::AddTypeConstructor( "Property", ConstructProperty );
	ScriptEngine::AddTypeDestructor( "Property", DestructProperty );

	ScriptEngine::AddTypeCopyConstructor( "Property", "Property", CopyConstructProperty<Property> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "string", CopyConstructProperty<std::string> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "float", CopyConstructProperty<float> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "Vector3D", CopyConstructProperty<Vector3D> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "uint64", CopyConstructProperty<uint64_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "uint", CopyConstructProperty<uint32_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "uint16", CopyConstructProperty<uint16_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "uint8", CopyConstructProperty<uint8_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "int64", CopyConstructProperty<int64_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "int", CopyConstructProperty<int32_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "int16", CopyConstructProperty<int16_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "int8", CopyConstructProperty<int8_t> );
	ScriptEngine::AddTypeCopyConstructor( "Property", "bool", CopyConstructProperty<bool> );

	ScriptEngine::AddTypeMethod( "Property", "PropertyType GetType()", &Property::GetType );
	ScriptEngine::AddTypeMethod( "Property", "string ToString()", &Property::ToString );
	ScriptEngine::AddTypeMethod( "Property", "const string GetString()", &Property::GetString );
	ScriptEngine::AddTypeMethod( "Property", "const float GetFloat()", &Property::GetFloat );
	ScriptEngine::AddTypeMethod( "Property", "const Vector3D GetVector3D()", &Property::GetVector3D );
	ScriptEngine::AddTypeMethod( "Property", "const int GetI32()", &Property::GetI32 );
}

void RegisterScriptFunctions()
{
	RegisterVector3Type();
	RegisterVector4Type();
	RegisterMath();
	RegisterSound();
	RegisterSequence();
	RegisterEntity();
	RegisterScriptEntity();
	RegisterTime();
	RegisterProperty();
}

void RegisterShatterEngine()
{
	RegisterScriptFunctions();
}