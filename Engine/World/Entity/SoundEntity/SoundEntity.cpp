// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoundEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

static CEntityFactory<CSoundEntity> Factory( "sound" );

CSoundEntity::CSoundEntity() : CPointEntity()
{
	Falloff = EFalloff::None;
	Radius = 0.0f;
	Volume = 100.0f;
	FadeIn = -1.0f;
	FadeOut = -1.0f;

	AutoPlay = false;
	AutoPlayed = false;
	Loop = false;

	Inputs["Play"] = [&] () { Play(); };
	Inputs["Stop"] = [&] () { Stop(); };
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
	UpdateSound();
}

void CSoundEntity::Tick()
{
	UpdateSound();
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
			Asset = Assets.FindSound( Property->Value );
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
			ExtractFloat( Property->Value.c_str(), Radius );
		}
		else if( Property->Key == "volume" )
		{
			ExtractFloat( Property->Value.c_str(), Volume );
		}
		else if( Property->Key == "fadein" )
		{
			ExtractFloat( Property->Value.c_str(), FadeIn );
		}
		else if( Property->Key == "fadeout" )
		{
			ExtractFloat( Property->Value.c_str(), FadeOut );
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
		else if( Property->Key == "3d" )
		{
			if( Property->Value == "0" )
			{
				Is3D = false;
			}
			else
			{
				Is3D = true;
			}
		}
	}

	Spawn( Transform );
}

void CSoundEntity::Play()
{
	if( Asset )
	{
		Sound = SoundInstance( Asset );
		
		Spatial Information;
		if( Is3D )
		{
			Information = Spatial::Create( Transform.GetPosition(), Vector3D::Zero );
		}
		else
		{
			Information.Bus = Bus::Music;
		}
		
		if( Falloff == EFalloff::None )
		{
			Information.Attenuation = Attenuation::Off;
		}
		else if( Falloff == EFalloff::Linear )
		{
			Information.Attenuation = Attenuation::Linear;
		}
		else if( Falloff == EFalloff::InverseSquare )
		{
			Information.Attenuation = Attenuation::Inverse;
		}

		Information.MinimumDistance = Radius;
		
		Information.FadeIn = FadeIn;

		// Start at volume 0 to avoid sounds from blasting loudly on the first tick.
		Information.Volume = 0.0f;
		
		Sound.Start( Information );
	}
}

void CSoundEntity::Stop()
{
	Sound.Stop( FadeOut );
}

void CSoundEntity::UpdateSound()
{
	if( !Asset )
		return;
	
	float Length = -1.0f;
	if( Falloff == EFalloff::Linear )
	{
		const auto* World = GetWorld();
		if( World )
		{
			auto* ActiveCamera = World->GetActiveCamera();
			if( ActiveCamera )
			{
				const auto& CameraSetup = ActiveCamera->GetCameraSetup();
				const Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

				Length = Delta.Length();
				Length /= Radius;

				OutOfRange = Length > 1.0f;
				
				Length = 1.0f - glm::clamp( Length, 0.0f, 1.0f );
			}
		}
	}
	else if( Falloff == EFalloff::InverseSquare )
	{
		const auto* World = GetWorld();
		if( World )
		{
			auto* ActiveCamera = World->GetActiveCamera();
			if( ActiveCamera )
			{
				const auto& CameraSetup = ActiveCamera->GetCameraSetup();
				const Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

				const float Factor = 1.0f / Radius;
				Length = Delta.Length() * Factor;
				
				Length = 1.0f / ( Length * Length );
				Length = glm::clamp( Length, 0.0f, 1.0f );

				OutOfRange = Length < 0.02f;
			}
		}
	}
	else
	{
		OutOfRange = false;
	}

	Range = Length;

	if( AutoPlay && !AutoPlayed && !OutOfRange )
	{
		Play();
		Sound.Loop( Loop );
		AutoPlayed = true;
	}

	if( OutOfRange )
	{
		if( Sound.Playing() )
		{
			Sound.Stop();
		}
		
		AutoPlayed = false;
	}
	else if( !Is3D && Length > 0.0f )
	{
		if( Sound.Playing() )
		{
			Sound.Volume( Length * Volume );
		}
	}
}

void CSoundEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );

	std::string SoundString;
	DataString::Decode( Data, SoundString );
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

	DataString::Encode( Data, SoundName.String() );
	Data << Falloff;
	Data << Radius;
	Data << Volume;
	Data << AutoPlay;
	Data << Loop;
}
