// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "SoundEntity.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>

static CEntityFactory<CSoundEntity> Factory( "sound" );

CSoundEntity::CSoundEntity()
{
	Transform = FTransform();
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
			const std::vector<float>& Coordinates = ExtractTokensFloat( Property->Value, ' ', 3 );
			if( Coordinates.size() == 3 )
			{
				Position = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "rotation" )
		{
			const std::vector<float>& Coordinates = ExtractTokensFloat( Property->Value, ' ', 3 );
			if( Coordinates.size() == 3 )
			{
				Orientation = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "scale" )
		{
			const std::vector<float>& Coordinates = ExtractTokensFloat( Property->Value, ' ', 3 );
			if( Coordinates.size() == 3 )
			{
				Size = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "sound" )
		{
			
		}
	}

	FTransform Transform( Position, Orientation, Size );
	Spawn( Transform );
}
