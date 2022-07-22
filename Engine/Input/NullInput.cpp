// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include "NullInput.h"

void CNullInput::RegisterKeyInput( EKey KeyInput, int ScanCode, EAction Action, int Modifiers )
{

}

void CNullInput::RegisterMouseButtonInput( EMouse MouseButton, EAction Action, int Modifiers )
{

}

void CNullInput::RegisterMousePositionInput( double PositionX, double PositionY )
{
	
}

void CNullInput::RegisterScrollInput( int OffsetX, int OffsetY )
{

}

void CNullInput::RegisterJoystickStatus( int Joystick, int Event )
{

}

void CNullInput::CreateActionBinding( const NameSymbol& ActionName )
{

}

void CNullInput::AddActionBinding( const NameSymbol& ActionName, const EKey& Key, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale )
{

}

void CNullInput::AddActionBinding( const NameSymbol& ActionName, const EMouse& Mouse, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale )
{

}

void CNullInput::AddActionBinding( const NameSymbol& ActionName, const EGamepad& Gamepad, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale )
{

}

void CNullInput::ClearActionBindings()
{

}

void CNullInput::Tick()
{

}

bool CNullInput::IsKeyDown( EKey KeyInput ) const
{
	return false;
}

bool CNullInput::IsMouseDown( EMouse MouseInput ) const
{
	return false;
}

bool CNullInput::IsAnyKeyDown() const
{
	return false;
}

FFixedPosition2D CNullInput::GetMousePosition() const
{
	return FFixedPosition2D();
}

void CNullInput::SetMousePosition( const FFixedPosition2D& Position )
{
	
}
