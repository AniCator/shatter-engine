// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoundEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

static CEntityFactory<CSoundEntity> Factory( "sound" );

CSoundEntity::CSoundEntity()
{
	Transform = FTransform();

	Falloff = EFalloff::None;
	Radius = 0.0f;

	AutoPlay = false;
	Loop = false;

	Inputs["Play"] = [this] () {this->Play(); };
}

CSoundEntity::CSoundEntity( FTransform& Transform ) : CEntity()
{
	Spawn( Transform );
}

CSoundEntity::~CSoundEntity()
{

}

void CSoundEntity::Spawn( FTransform& Transform )
{
	this->Transform = Transform;
}

void CSoundEntity::Construct()
{
	
}

void CSoundEntity::Tick()
{
	if( Sound )
	{
		if( AutoPlay )
		{
			AutoPlay = false;
			Sound->Start();
			Sound->Loop( Loop );
		}

		if( Falloff == EFalloff::None )
		{
			Sound->Volume( 100.0f );
		}
		else if( Falloff == EFalloff::Linear )
		{
			auto World = GetWorld();
			if( World )
			{
				const auto& CameraSetup = World->GetActiveCameraSetup();
				glm::vec3 Delta = CameraSetup.CameraPosition - Transform.GetPosition();

				float Length = Math::Length( Delta );
				Length /= Radius;
				Length = glm::clamp( Length, 0.0f, 1.0f );

				Sound->Volume( Length * 100.0f );
			}
		}
		else if( Falloff == EFalloff::InverseSquare )
		{
			auto World = GetWorld();
			if( World )
			{
				const auto& CameraSetup = World->GetActiveCameraSetup();
				glm::vec3 Delta = CameraSetup.CameraPosition - Transform.GetPosition();

				const float Factor = 1.0f / Radius;
				float Length = Math::Length( Delta ) * Factor;
				Length = 1.0f / ( Length * Length );
				Length = glm::clamp( Length, 0.0f, 1.0f );

				Sound->Volume( Length * 100.0f );
			}
		}
	}
}

void CSoundEntity::Destroy()
{
	Sound->Stop();
}

void CSoundEntity::Load( const JSON::Vector& Objects )
{
	CAssets& Assets = CAssets::Get();

	glm::vec3 Position;
	glm::vec3 Orientation;
	glm::vec3 Size;

	for( auto Property : Objects )
	{
		if( Property->Key == "position" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Position = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "rotation" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Orientation = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "scale" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Size = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "sound" )
		{
			Sound = Assets.FindSound( Property->Value );
		}
		else if( Property->Key == "falloff" )
		{
			if( Property->Value == "linear" )
			{
				Falloff = EFalloff::Linear;
			}
			else if( Property->Value == "inversesqr" )
			{
				Falloff = EFalloff::InverseSquare;
			}
		}
		else if( Property->Key == "radius" )
		{
			size_t OutTokenCount = 0;
			auto TokenDistance = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 1 );
			if( OutTokenCount == 1 )
			{
				Radius = TokenDistance[0];
			}
		}
		else if( Property->Key == "autoplay" )
		{
			if( Property->Value == "1" )
			{
				AutoPlay = true;
			}
		}
		else if( Property->Key == "loop" )
		{
			if( Property->Value == "1" )
			{
				Loop = true;
			}
		}
	}

	FTransform Transform( Position, Orientation, Size );
	Spawn( Transform );
}

void CSoundEntity::Play()
{
	if( Sound )
	{
		Sound->Start();
	}
}

void CSoundEntity::Stop()
{
	if( Sound )
	{
		Sound->Stop();
	}
}
