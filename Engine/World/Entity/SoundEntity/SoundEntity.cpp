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
	AutoPlayed = false;
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
	AutoPlayed = false;
}

void CSoundEntity::Tick()
{
	if( Sound )
	{
		if( Falloff == EFalloff::None )
		{
			Sound->Volume( Volume );
		}
		else if( Falloff == EFalloff::Linear )
		{
			auto World = GetWorld();
			if( World )
			{
				auto ActiveCamera = World->GetActiveCamera();
				if( ActiveCamera )
				{
					const auto& CameraSetup = ActiveCamera->GetCameraSetup();
					Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

					float Length = Delta.Length();
					Length /= Radius;
					Length = glm::clamp( Length, 0.0f, 1.0f );

					Sound->Volume( Length * Volume );
				}
			}
		}
		else if( Falloff == EFalloff::InverseSquare )
		{
			auto World = GetWorld();
			if( World )
			{
				auto ActiveCamera = World->GetActiveCamera();
				if( ActiveCamera )
				{
					const auto& CameraSetup = ActiveCamera->GetCameraSetup();
					Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

					const float Factor = 1.0f / Radius;
					float Length = Delta.Length() * Factor;
					Length = 1.0f / ( Length * Length );
					Length = glm::clamp( Length, 0.0f, 1.0f );

					Sound->Volume( Length * Volume );
				}
			}
		}

		if( AutoPlay && !AutoPlayed )
		{
			AutoPlayed = true;
			Sound->Start();
			Sound->Loop( Loop );
		}
	}
}

void CSoundEntity::Destroy()
{
	Stop();
}

void CSoundEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );
	CAssets& Assets = CAssets::Get();

	for( auto Property : Objects )
	{
		if( Property->Key == "sound" )
		{
			SoundName = Property->Value;
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
			auto TokenDistance = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, 1 );
			if( OutTokenCount == 1 )
			{
				Radius = TokenDistance[0];
			}
		}
		else if( Property->Key == "volume" )
		{
			size_t OutTokenCount = 0;
			auto TokenDistance = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, 1 );
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

void CSoundEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );

	std::string SoundString;
	FDataString::Decode( Data, SoundString );
	SoundName = SoundString;

	Sound = CAssets::Get().FindSound( SoundString );

	Data >> Falloff;
	Data >> Radius;
	Data >> Volume;
	Data >> AutoPlay;
	Data >> Loop;
}

void CSoundEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );

	FDataString::Encode( Data, SoundName.String() );
	Data << Falloff;
	Data << Radius;
	Data << Volume;
	Data << AutoPlay;
	Data << Loop;
}
