// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoundEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>

static CEntityFactory<CSoundEntity> Factory( "sound" );

CSoundEntity::CSoundEntity()
{
	Transform = FTransform();

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
	
}

void CSoundEntity::Destroy()
{

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
