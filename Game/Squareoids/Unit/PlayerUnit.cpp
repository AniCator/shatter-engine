// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "PlayerUnit.h"

#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Math.h>

#include <Engine/Profiling/Profiling.h>

CSquareoidsPlayerUnit::CSquareoidsPlayerUnit()
{
	UnitData.Position = glm::vec3( 0.0f, 0.0f, 0.0f );
	UnitData.Velocity = glm::vec3( 0.0f, 0.0f, 0.0f );
	UnitData.Color = glm::vec4( 0.0f, 0.0f, 1.0f, 1.0f );
	UnitData.Size = glm::vec3( 5.0f, 5.0f, 5.0f );

	UnitData.Health = 5.0f;
	Absorbing = false;

	IInput& Input = CInputLocator::GetService();
	Input.AddActionBinding( EActionBindingType::Keyboard, 87, 1, std::bind( &CSquareoidsPlayerUnit::InputForwardPress, this ) );
	Input.AddActionBinding( EActionBindingType::Keyboard, 87, 0, std::bind( &CSquareoidsPlayerUnit::InputForwardRelease, this ) );

	Input.AddActionBinding( EActionBindingType::Keyboard, 83, 1, std::bind( &CSquareoidsPlayerUnit::InputBackwardPress, this ) );
	Input.AddActionBinding( EActionBindingType::Keyboard, 83, 0, std::bind( &CSquareoidsPlayerUnit::InputBackwardRelease, this ) );

	Input.AddActionBinding( EActionBindingType::Keyboard, 65, 1, std::bind( &CSquareoidsPlayerUnit::InputLeftPress, this ) );
	Input.AddActionBinding( EActionBindingType::Keyboard, 65, 0, std::bind( &CSquareoidsPlayerUnit::InputLeftRelease, this ) );

	Input.AddActionBinding( EActionBindingType::Keyboard, 68, 1, std::bind( &CSquareoidsPlayerUnit::InputRightPress, this ) );
	Input.AddActionBinding( EActionBindingType::Keyboard, 68, 0, std::bind( &CSquareoidsPlayerUnit::InputRightRelease, this ) );
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

		const glm::vec3 InteractionBoundingMinimum = InteractionUnitData.Position - InteractionUnitData.Size;
		const glm::vec3 InteractionBoundingMaximum = InteractionUnitData.Position + InteractionUnitData.Size;

		const glm::vec3 BoundingMinimum = UnitData.Position - UnitData.Size;
		const glm::vec3 BoundingMaximum = UnitData.Position + UnitData.Size;

		if( Math::BoundingBoxIntersection( InteractionBoundingMinimum, InteractionBoundingMaximum, BoundingMinimum, BoundingMaximum ) )
		{
			const float AbsorptionRatio = ( InteractionUnitData.Size[0] / UnitData.Size[0] );
			glm::vec3 Direction3D = InteractionUnitData.Position - UnitData.Position;
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

	const float Scale = 20.0f;
	const float OffsetX = static_cast<float>( Left + Right ) * Scale;
	const float OffsetY = static_cast<float>( Forward + Back ) * Scale;

	const float InterpolationFactor = 0.1f;
	const float OneMinusInterp = 1.0f - InterpolationFactor;
	UnitData.Velocity[0] = ( UnitData.Velocity[0] * OneMinusInterp ) + (OffsetX * InterpolationFactor );
	UnitData.Velocity[1] = ( UnitData.Velocity[1] * OneMinusInterp ) + (OffsetY * InterpolationFactor );

	UnitData.Position[0] += UnitData.Velocity[0];
	UnitData.Position[1] += UnitData.Velocity[1];

	UnitData.Size = glm::vec3( UnitData.Health * 0.5f, UnitData.Health * 0.5f, UnitData.Health * 0.5f );

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
	CProfileVisualisation& Profiler = CProfileVisualisation::GetInstance();

	char PositionXString[32];
	sprintf_s( PositionXString, "%f", UnitData.Position[0] );
	char PositionYString[32];
	sprintf_s( PositionYString, "%f", UnitData.Position[1] );

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
