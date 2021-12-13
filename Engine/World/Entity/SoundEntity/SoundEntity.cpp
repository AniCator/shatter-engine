// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoundEntity.h"

#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

static CEntityFactory<SoundEntity> Factory( "sound" );

SoundEntity::SoundEntity() : CPointEntity()
{
	Falloff = EFalloff::None;
	Radius = 5.0f;
	Volume = 100.0f;
	FadeIn = -1.0f;
	FadeOut = -1.0f;

	AutoPlay = false;
	AutoPlayed = false;
	Loop = false;

	Inputs["Play"] = [&] ( CEntity* Origin )
	{
		Play();

		return true;
	};

	Inputs["Stop"] = [&] ( CEntity* Origin )
	{
		Stop();

		return true;
	};

}

SoundEntity::SoundEntity( FTransform& Transform ) : CPointEntity()
{
	Spawn( Transform );
}

SoundEntity::~SoundEntity()
{

}

void SoundEntity::Spawn( FTransform& Transform )
{
	this->Transform = Transform;
}

void SoundEntity::Construct()
{
	AutoPlayed = false;
	UpdateSound();
}

void SoundEntity::Tick()
{
	UpdateSound();
}

void SoundEntity::Destroy()
{
	Stop();
}

void SoundEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );
	CAssets& Assets = CAssets::Get();

	bool BusFound = false;
	for( auto* Property : Objects )
	{
		if( Property->Key == "sound" )
		{
			SoundName = Property->Value;
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
			Extract( Property->Value, Radius );
		}
		else if( Property->Key == "rate" )
		{
			Extract( Property->Value, Rate );
		}
		else if( Property->Key == "volume" )
		{
			Extract( Property->Value, Volume );
		}
		else if( Property->Key == "fadein" )
		{
			Extract( Property->Value, FadeIn );
		}
		else if( Property->Key == "fadeout" )
		{
			Extract( Property->Value, FadeOut );
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
		else if( Property->Key == "bus" )
		{
			int32_t BusInteger = Bus::SFX;
			Extract( Property->Value, BusInteger );
			Bus = Bus::Type( BusInteger );
			BusFound = true;
		}
	}

	// If no bus was specified, set it to the Music channel if the sound isn't spatialized.
	if( !BusFound && !Is3D )
	{
		Bus = Bus::Music;
	}

	Spawn( Transform );
}

void SoundEntity::Reload()
{
	Asset = CAssets::Get().FindSound( SoundName );

	CPointEntity::Reload();
}

void SoundEntity::Play()
{
	if( Asset )
	{
		Sound = SoundInstance( Asset );
		
		Spatial Information;
		if( Is3D )
		{
			Information = Spatial::Create( Transform.GetPosition(), Vector3D::Zero );
		}

		Information.Bus = Bus;
		
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
		Information.Volume = Volume;

		Information.Rate = Rate;
		
		Sound.Start( Information );
	}
}

void SoundEntity::Stop()
{
	Sound.Stop( FadeOut );
}

void SoundEntity::UpdateSound()
{
	if( !Asset )
		return;

	const auto* World = GetWorld();
	if( !World )
		return;
	
	auto* ActiveCamera = World->GetActiveCamera();
	if( !ActiveCamera )
		return;
	
	float Length = -1.0f;
	if( !Is3D ) // We only need to calculate the length if it's not handled by SoLoud.
	{
		if( Falloff == EFalloff::Linear )
		{
			const auto& CameraSetup = ActiveCamera->GetCameraSetup();
			const Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

			Length = Delta.Length();
			Length /= Radius;

			OutOfRange = Length > 1.0f;

			Length = 1.0f - glm::clamp( Length, 0.0f, 1.0f );
		}
		else if( Falloff == EFalloff::InverseSquare )
		{
			const auto& CameraSetup = ActiveCamera->GetCameraSetup();
			const Vector3D Delta = CameraSetup.CameraPosition - Transform.GetPosition();

			const float Factor = 1.0f / Radius;
			Length = Delta.Length() * Factor;

			Length = 1.0f / ( Length * Length );
			Length = glm::clamp( Length, 0.0f, 1.0f );

			OutOfRange = Length < 0.02f;
		}
		else
		{
			OutOfRange = false;
			Length = 1.0f;
		}

		Range = Length;
	}

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

void SoundEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );

	DataString::Decode( Data, SoundName );
	Data >> Falloff;
	Data >> Radius;
	Data >> Rate;
	Data >> Volume;
	Data >> AutoPlay;
	Data >> Loop;
	Data >> Is3D;
	Data >> Bus;
}

void SoundEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );

	DataString::Encode( Data, SoundName );
	Data << Falloff;
	Data << Radius;
	Data << Rate;
	Data << Volume;
	Data << AutoPlay;
	Data << Loop;
	Data << Is3D;
	Data << Bus;
}
