// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PointEntity.h"

#include <Engine/Audio/Sound.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/World.h>

#include <Engine/Display/UserInterface.h>

CPointEntity::CPointEntity()
{
	Transform = FTransform();
}

CPointEntity::CPointEntity( const FTransform& Transform ) : CEntity()
{
	this->Transform = Transform;
	ShouldUpdateTransform = true;
}

CPointEntity::~CPointEntity()
{

}

const FTransform& CPointEntity::GetTransform()
{
	if( ShouldUpdateTransform )
	{
		if( Level && false )
		{
			WorldTransform = Level->GetTransform().Transform( Transform );
		}
		else
		{
			WorldTransform = Transform;
		}

		if( Parent )
		{
			auto Entity = dynamic_cast<CPointEntity*>( Parent );
			if( Entity )
			{
				auto& ParentTransform = Entity->GetTransform();
			}
		}

		ShouldUpdateTransform = false;
	}

	return WorldTransform;
}

const FTransform& CPointEntity::GetLocalTransform()
{
	return Transform;
}

void CPointEntity::SetTransform(const FTransform& TransformIn )
{
	Transform = TransformIn;
	ShouldUpdateTransform = true;
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
			auto Coordinates = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Position = { Coordinates[0], Coordinates[1], Coordinates[2] };
			}
		}
		else if( Property->Key == "rotation" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Orientation = { Coordinates[0], Coordinates[1], Coordinates[2] };
			}
		}
		else if( Property->Key == "scale" )
		{
			size_t OutTokenCount = 0;
			auto Coordinates = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, 3 );
			if( OutTokenCount == 3 )
			{
				Size = { Coordinates[0], Coordinates[1], Coordinates[2] };
			}
		}
	}

	ShouldUpdateTransform = true;

	Transform.SetTransform( Position, Orientation, Size );

	if( Level )
	{
		Transform = Level->GetTransform().Transform( Transform );
	}
}

void CPointEntity::Debug()
{
	UI::AddCircle( Transform.GetPosition(), 2.0f, Color::White );
	UI::AddText( Transform.GetPosition() - Vector3D( 0.0, 0.0, -1.0f ), Name.String().c_str() );
}

void CPointEntity::Import( CData& Data )
{
	Data >> Transform;
	Data >> WorldTransform;

	ShouldUpdateTransform = true;
}

void CPointEntity::Export( CData& Data )
{
	Data << Transform;
	Data << WorldTransform;
}
