// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include "NullInput.h"

void CNullInput::RegisterKeyInput( int KeyInput, int ScanCode, int Action, int Modifiers )
{

}

void CNullInput::RegisterMouseButtonInput( int MouseButton, int Action, int Modifiers )
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

void CNullInput::AddActionBinding( EActionBindingType BindingType, int KeyInput, int Action, ActionTarget TargetFunc )
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
