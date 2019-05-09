// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PointEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

CPointEntity::CPointEntity()
{
	Transform = FTransform();
}

CPointEntity::CPointEntity( const FTransform& Transform ) : CEntity()
{
	this->Transform = Transform;
}

CPointEntity::~CPointEntity()
{

}

void CPointEntity::Load( const JSON::Vector& Objects )
{
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
	}

	if( Level )
	{
		auto Transform = Level->GetTransform();
		auto NewPosition = Transform.Position( Vector3DToGLM( Position ) );
		Position = { NewPosition[0], NewPosition[1], NewPosition[2] };
	}

	Transform.SetTransform( Position, Orientation, Size );
}
