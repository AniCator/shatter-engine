// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Input/InputInterface.h>

#include <vector>

static const EKeyType MaximumKeyboardInputs = static_cast<EKeyType>( EKey::Maximum );
static const int MaximumJoysticks = 16;
static const int MaximumMouseButtons = 8;

struct FKeyInput
{
	EKey Input;
	int Action;
	int Modifiers;
};

struct FMouseInput
{
	int Input;
	int Action;
	int Modifiers;
};

class CInput : public IInput
{
public:
	CInput();

	CInput( CInput const& ) = delete;
	void operator=( CInput const& ) = delete;

	virtual void RegisterKeyInput( EKey KeyInput, int ScanCode, int Action, int Modifiers ) override;
	virtual void RegisterMouseButtonInput( int MouseButton, int Action, int Modifiers ) override;
	virtual void RegisterMousePositionInput( double PositionX, double PositionY ) override;
	virtual void RegisterScrollInput( int OffsetX, int OffsetY ) override;
	virtual void RegisterJoystickStatus( int Joystick, int Event ) override;

	virtual void AddActionBinding( FActionBinding ActionBinding ) override;
	virtual void AddActionBinding( EActionBindingType BindingType, EKey KeyInput, int Action, ActionTarget TargetFunc ) override;
	virtual void ClearActionBindings() override;

	virtual void Tick() override;

	virtual bool IsKeyDown( int KeyInput ) const override;
	virtual bool IsAnyKeyDown() const override;
	virtual FFixedPosition2D GetMousePosition() const override;

private:
	std::vector<FActionBinding> ActionBindings;

	// Keyboard Inputs
	FKeyInput KeyboardInput[MaximumKeyboardInputs];

	// Joystick states
	int Joystick[MaximumJoysticks];

	// Mouse button states
	FMouseInput MouseInput[MaximumMouseButtons];

	FFixedPosition2D MousePosition;

	// Any state
	bool AnyKey;
};
