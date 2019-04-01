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

void CNullInput::AddActionBinding( FActionBinding ActionBinding )
{

}

void CNullInput::AddActionBinding( EKey KeyInput, EAction Action, ActionTarget TargetFunc )
{

}

void CNullInput::AddActionBinding( EMouse KeyInput, EAction Action, ActionTarget TargetFunc )
{

}

void CNullInput::AddActionBinding( EGamepad KeyInput, EAction Action, ActionTarget TargetFunc )
{

}

void CNullInput::ClearActionBindings()
{

}

void CNullInput::Tick()
{

}

bool CNullInput::IsKeyDown( int KeyInput ) const
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
