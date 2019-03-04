// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PlayerUnit.h"

#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Math.h>

#include <Engine/Profiling/Profiling.h>

CSquareoidsPlayerUnit::CSquareoidsPlayerUnit()
{
	UnitData.Transform = { glm::vec3( 0.0f, 0.0f, 0.0f ), WorldUp, glm::vec3( 5.0f ) };
	UnitData.Velocity = glm::vec3( 0.0f, 0.0f, 0.0f );
	UnitData.Color = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );

	UnitData.Health = 5.0f;
	Absorbing = false;

	IInput& Input = CInputLocator::GetService();
	Input.AddActionBinding( EKey::W, EAction::Press, std::bind( &CSquareoidsPlayerUnit::InputForwardPress, this ) );
	Input.AddActionBinding( EKey::W, EAction::Release, std::bind( &CSquareoidsPlayerUnit::InputForwardRelease, this ) );

	Input.AddActionBinding( EKey::S, EAction::Press, std::bind( &CSquareoidsPlayerUnit::InputBackwardPress, this ) );
	Input.AddActionBinding( EKey::S, EAction::Release, std::bind( &CSquareoidsPlayerUnit::InputBackwardRelease, this ) );

	Input.AddActionBinding( EKey::A, EAction::Press, std::bind( &CSquareoidsPlayerUnit::InputLeftPress, this ) );
	Input.AddActionBinding( EKey::A, EAction::Release, std::bind( &CSquareoidsPlayerUnit::InputLeftRelease, this ) );

	Input.AddActionBinding( EKey::D, EAction::Press, std::bind( &CSquareoidsPlayerUnit::InputRightPress, this ) );
	Input.AddActionBinding( EKey::D, EAction::Release, std::bind( &CSquareoidsPlayerUnit::InputRightRelease, this ) );
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

		const glm::vec3 PositionA = UnitData.Transform.GetPosition();
		const glm::vec3 PositionB = InteractionUnitData.Transform.GetPosition();

		const glm::vec3 SizeA = UnitData.Transform.GetSize();
		const glm::vec3 SizeB = InteractionUnitData.Transform.GetSize();

		const glm::vec3 InteractionBoundingMinimum = PositionB - SizeB;
		const glm::vec3 InteractionBoundingMaximum = PositionB + SizeB;

		const glm::vec3 BoundingMinimum = PositionA - SizeA;
		const glm::vec3 BoundingMaximum = PositionA + SizeA;

		if( Math::BoundingBoxIntersection( InteractionBoundingMinimum, InteractionBoundingMaximum, BoundingMinimum, BoundingMaximum ) )
		{
			const float AbsorptionRatio = ( SizeB[0] / SizeA[0] );
			glm::vec3 Direction3D = PositionB - PositionA;
			InteractionUnitData.Velocity += Direction3D * 0.1f;

			InteractionUnitData.Health -= AbsorptionRatio;
			UnitData.Health += AbsorptionRatio;

			Absorbing = true;
		}
	}
}

void CSquareoidsPlayerUnit::Tick()
{
	const int Forward = Inputs & ESquareoidsInputFlag::Forward ? 1 : 0;
	const int Back = Inputs & ESquareoidsInputFlag::Backward ? -1 : 0;
	const int Left = Inputs & ESquareoidsInputFlag::Left ? -1 : 0;
	const int Right = Inputs & ESquareoidsInputFlag::Right ? 1 : 0;

	glm::vec3 Position = UnitData.Transform.GetPosition();

	const float Scale = 20.0f;
	const float OffsetX = static_cast<float>( Left + Right ) * Scale;
	const float OffsetY = static_cast<float>( Forward + Back ) * Scale;

	const float InterpolationFactor = 0.1f;
	const float OneMinusInterp = 1.0f - InterpolationFactor;
	UnitData.Velocity[0] = ( UnitData.Velocity[0] * OneMinusInterp ) + (OffsetX * InterpolationFactor );
	UnitData.Velocity[1] = ( UnitData.Velocity[1] * OneMinusInterp ) + (OffsetY * InterpolationFactor );

	Position[0] += UnitData.Velocity[0];
	Position[1] += UnitData.Velocity[1];
	Position[2] = 100.0f;

	UnitData.Transform.SetTransform( Position, UnitData.Transform.GetOrientation(), glm::vec3( UnitData.Health * 0.5f ) );

	if( Absorbing )
	{
		UnitData.Color = glm::vec4( 0.0f, 1.0f, 1.0f, 1.0f );
		Absorbing = false;
	}
	else
	{
		UnitData.Color = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
	}

	// Visualize in profiler
	CProfiler& Profiler = CProfiler::Get();

	char PositionXString[32];
	sprintf_s( PositionXString, "%f", Position[0] );
	char PositionYString[32];
	sprintf_s( PositionYString, "%f", Position[1] );

	Profiler.AddDebugMessage( "PlayerPositionX", PositionXString );
	Profiler.AddDebugMessage( "PlayerPositionY", PositionYString );

	char OffsetXString[32];
	sprintf_s( OffsetXString, "%f", OffsetX );
	char OffsetYString[32];
	sprintf_s( OffsetYString, "%f", OffsetY );

	Profiler.AddDebugMessage( "OffsetX", OffsetXString );
	Profiler.AddDebugMessage( "OffsetY", OffsetYString );

	char HealthString[32];
	sprintf_s( HealthString, "%f", UnitData.Health );
	Profiler.AddDebugMessage( "Health", HealthString );
}

FSquareoidUnitData& CSquareoidsPlayerUnit::GetUnitData()
{
	return UnitData;
}

void CSquareoidsPlayerUnit::InputForwardPress()
{
	Inputs |= ESquareoidsInputFlag::Forward;
}

void CSquareoidsPlayerUnit::InputForwardRelease()
{
	Inputs &= ~ESquareoidsInputFlag::Forward;
}

void CSquareoidsPlayerUnit::InputBackwardPress()
{
	Inputs |= ESquareoidsInputFlag::Backward;
}

void CSquareoidsPlayerUnit::InputBackwardRelease()
{
	Inputs &= ~ESquareoidsInputFlag::Backward;
}

void CSquareoidsPlayerUnit::InputLeftPress()
{
	Inputs |= ESquareoidsInputFlag::Left;
}

void CSquareoidsPlayerUnit::InputLeftRelease()
{
	Inputs &= ~ESquareoidsInputFlag::Left;
}

void CSquareoidsPlayerUnit::InputRightPress()
{
	Inputs |= ESquareoidsInputFlag::Right;
}

void CSquareoidsPlayerUnit::InputRightRelease()
{
	Inputs &= ~ESquareoidsInputFlag::Right;
}
