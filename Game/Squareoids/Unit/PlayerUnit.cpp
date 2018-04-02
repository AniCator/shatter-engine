// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PlayerUnit.h"

// TODO: Checks inputs directly for now
#include <Engine/Input/Input.h>

#include <Engine/Profiling/Profiling.h>

CSquareoidsPlayerUnit::CSquareoidsPlayerUnit()
{
	UnitData.Color = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
	UnitData.Position = glm::vec3( 0.0f, 0.0f, 0.0f );
	UnitData.Size = glm::vec3( 5.0f, 5.0f, 5.0f );
}

CSquareoidsPlayerUnit::~CSquareoidsPlayerUnit()
{

}

void CSquareoidsPlayerUnit::Interaction( ISquareoidsUnit* Unit )
{
	CSquareoidsUnit* CastUnit = static_cast<CSquareoidsUnit*>( Unit );
	if( CastUnit )
	{
		FSquareoidUnitData& InteractionUnitData = CastUnit->GetUnitData();
	}
}

void CSquareoidsPlayerUnit::Tick()
{
	CInput& Input = CInput::GetInstance();

	const int Forward = Input.IsKeyDown( 87 ) ? 1 : 0;
	const int Back = Input.IsKeyDown( 83 ) ? -1 : 0;
	const int Left = Input.IsKeyDown( 65 ) ? -1 : 0;
	const int Right = Input.IsKeyDown( 68 ) ? 1 : 0;

	const float Scale = 1000.0f;
	const float OffsetX = static_cast<float>( Left + Right ) * Scale;
	const float OffsetY = static_cast<float>( Forward + Back ) * Scale;

	const float InterpolationFactor = 0.1f;
	const float OneMinusInterp = 1.0f - InterpolationFactor;
	UnitData.Position[0] = ( UnitData.Position[0] * OneMinusInterp ) + ( OffsetX * InterpolationFactor );
	UnitData.Position[1] = ( UnitData.Position[1] * OneMinusInterp ) + ( OffsetY * InterpolationFactor );

	CProfileVisualisation& Profiler = CProfileVisualisation::GetInstance();

	char PositionXString[32];
	sprintf_s( PositionXString, "%f", UnitData.Position[0] );
	char PositionYString[32];
	sprintf_s( PositionYString, "%f", UnitData.Position[1] );

	Profiler.AddDebugMessage( "PlayerPositionX", PositionXString );
	Profiler.AddDebugMessage( "PlayerPositionY", PositionYString );
}

FSquareoidUnitData& CSquareoidsPlayerUnit::GetUnitData()
{
	return UnitData;
}
