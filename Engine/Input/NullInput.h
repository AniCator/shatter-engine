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

	virtual void CreateActionBinding( const NameSymbol& ActionName ) override;
	virtual void AddActionBinding( const NameSymbol& ActionName, const EKey& Key, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	virtual void AddActionBinding( const NameSymbol& ActionName, const EMouse& Mouse, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	virtual void AddActionBinding( const NameSymbol& ActionName, const EGamepad& Gamepad, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	virtual void ClearActionBindings() override;

	virtual void Tick() override;

	virtual bool IsKeyDown( EKey KeyInput ) const override;
	virtual bool IsMouseDown( EMouse MouseInput ) const override;
	virtual bool IsAnyKeyDown() const override;
	virtual FFixedPosition2D GetMousePosition() const override;
	virtual void SetMousePosition( const FFixedPosition2D& Position ) override;

};
