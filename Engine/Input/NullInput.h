// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Input/InputInterface.h>

class CNullInput : public IInput
{
public:
	virtual void RegisterKeyInput( EKey KeyInput, int ScanCode, EAction Action, int Modifiers ) override;
	virtual void RegisterMouseButtonInput( EMouse MouseButton, EAction Action, int Modifiers ) override;
	virtual void RegisterMousePositionInput( double PositionX, double PositionY ) override;
	virtual void RegisterScrollInput( int OffsetX, int OffsetY ) override;
	virtual void RegisterJoystickStatus( int Joystick, int Event ) override;

	virtual void AddActionBinding( FActionBinding ActionBinding ) override;
	virtual void AddActionBinding( EKey KeyInput, EAction Action, ActionTarget TargetFunc ) override;
	virtual void AddActionBinding( EMouse KeyInput, EAction Action, ActionTarget TargetFunc ) override;
	virtual void AddActionBinding( EGamepad KeyInput, EAction Action, ActionTarget TargetFunc ) override;
	virtual void ClearActionBindings() override;

	virtual void Tick() override;

	virtual bool IsKeyDown( int KeyInput ) const override;
	virtual bool IsAnyKeyDown() const override;
	virtual FFixedPosition2D GetMousePosition() const override;
	virtual void SetMousePosition( const FFixedPosition2D& Position ) override;

};
