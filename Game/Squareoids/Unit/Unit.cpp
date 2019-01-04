// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Unit.h"

#include <Engine/Utility/Math.h>

CSquareoidsUnit::CSquareoidsUnit()
{
	UnitData.Position = glm::vec3( 0.0f, 0.0f, 0.0f );
	UnitData.Velocity = glm::vec3( 0.0f, 0.0f, 0.0f );
	UnitData.Color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	UnitData.Size = glm::vec3( 5.0f, 5.0f, 5.0f );

	UnitData.Health = 5.0f + static_cast<float>( std::rand() ) / RAND_MAX * 45.0f;
}

CSquareoidsUnit::~CSquareoidsUnit()
{

}

void CSquareoidsUnit::Interaction( ISquareoidsUnit* Unit )
{
	/*CSquareoidsUnit* CastUnit = static_cast<CSquareoidsUnit*>( Unit );
	if( CastUnit )
	{
		FSquareoidUnitData& InteractionUnitData = CastUnit->GetUnitData();

		const glm::vec3 InteractionBoundingMinimum = InteractionUnitData.Position - InteractionUnitData.Size;
		const glm::vec3 InteractionBoundingMaximum = InteractionUnitData.Position + InteractionUnitData.Size;

		const glm::vec3 BoundingMinimum = UnitData.Position - UnitData.Size;
		const glm::vec3 BoundingMaximum = UnitData.Position + UnitData.Size;

		if( Math::BoundingBoxIntersection( InteractionBoundingMinimum, InteractionBoundingMaximum, BoundingMinimum, BoundingMaximum ) )
		{
			glm::vec3 Direction3D = InteractionUnitData.Position - UnitData.Position;
			InteractionUnitData.Velocity += Direction3D;
			UnitData.Velocity = glm::vec3( 0.0f, 0.0f, 0.0f );
		}
	}*/
}

void CSquareoidsUnit::Tick()
{
	UnitData.Size = glm::vec3( UnitData.Health * 0.5f, UnitData.Health * 0.5f, UnitData.Health * 0.5f );

	UnitData.Position += UnitData.Velocity;
	UnitData.Position[2] = 100.0f;
	UnitData.Velocity *= 0.33f;
}

FSquareoidUnitData& CSquareoidsUnit::GetUnitData()
{
	return UnitData;
}
