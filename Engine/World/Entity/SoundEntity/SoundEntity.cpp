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
	Volume = 100.0f;

	AutoPlay = false;
	Loop = false;

	Inputs["Play"] = [this] () {this->Play(); };
}

CSoundEntity::CSoundEntity( FTransform& Transform ) : CPointEntity()
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
			Sound->Volume( Volume );
		}
		else if( Falloff == EFalloff::Linear )
		{
			auto World = GetWorld();
			if( World )
			{
				const auto& CameraSetup = World->GetActiveCameraSetup();
				Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

				float Length = Delta.Length();
				Length /= Radius;
				Length = glm::clamp( Length, 0.0f, 1.0f );

				Sound->Volume( Length * Volume );
			}
		}
		else if( Falloff == EFalloff::InverseSquare )
		{
			auto World = GetWorld();
			if( World )
			{
				const auto& CameraSetup = World->GetActiveCameraSetup();
				Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

				const float Factor = 1.0f / Radius;
				float Length = Delta.Length() * Factor;
				Length = 1.0f / ( Length * Length );
				Length = glm::clamp( Length, 0.0f, 1.0f );

				Sound->Volume( Length * Volume );
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
	CPointEntity::Load( Objects );
	CAssets& Assets = CAssets::Get();

	Vector3D Position;
	Vector3D Orientation;
	Vector3D Size;

	for( auto Property : Objects )
	{
		if( Property->Key == "position" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Position = { Coordinates[0], Coordinates[1], Coordinates[2] };
			}
		}
		else if( Property->Key == "rotation" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Orientation = { Coordinates[0], Coordinates[1], Coordinates[2] };
			}
		}
		else if( Property->Key == "scale" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Size = { Coordinates[0], Coordinates[1], Coordinates[2] };
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
		else if( Property->Key == "volume" )
		{
			size_t OutTokenCount = 0;
			auto TokenDistance = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, 1 );
			if( OutTokenCount == 1 )
			{
				Volume = TokenDistance[0];
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
