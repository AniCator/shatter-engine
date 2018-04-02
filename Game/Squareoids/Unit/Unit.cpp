// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Unit.h"

CSquareoidsUnit::CSquareoidsUnit()
{
	UnitData.Color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	UnitData.Position = glm::vec3( 0.0f, 0.0f, 100.0f );
	UnitData.Size = glm::vec3( 5.0f, 5.0f, 5.0f );
}

CSquareoidsUnit::~CSquareoidsUnit()
{

}

void CSquareoidsUnit::Interaction( ISquareoidsUnit* Unit )
{
	CSquareoidsUnit* CastUnit = static_cast<CSquareoidsUnit*>( Unit );
	if( CastUnit )
	{
		FSquareoidUnitData& InteractionUnitData = CastUnit->GetUnitData();
	}
}

void CSquareoidsUnit::Tick()
{
	
}

FSquareoidUnitData& CSquareoidsUnit::GetUnitData()
{
	return UnitData;
}
