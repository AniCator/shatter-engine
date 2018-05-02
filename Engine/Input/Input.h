// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Input/InputInterface.h>

#include <vector>

static const int MaximumKeyboardInputs = 512;
static const int MaximumJoysticks = 16;
static const int MaximumMouseButtons = 8;

struct FInput
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

	void RegisterKeyInput( int KeyInput, int ScanCode, int Action, int Modifiers );
	void RegisterMouseButtonInput( int MouseButton, int Action, int Modifiers );
	void RegisterScrollInput( int OffsetX, int OffsetY );
	void RegisterJoystickStatus( int Joystick, int Event );

	void AddActionBinding( FActionBinding ActionBinding );
	void AddActionBinding( EActionBindingType BindingType, int KeyInput, int Action, ActionTarget TargetFunc );
	void ClearActionBindings();

	void Tick();

	bool IsKeyDown( int KeyInput ) const;
	bool IsAnyKeyDown() const;

private:
	std::vector<FActionBinding> ActionBindings;

	// Keyboard Inputs
	FInput KeyboardInput[MaximumKeyboardInputs];

	// Joystick states
	int Joystick[MaximumJoysticks];

	// Mouse button states
	FInput MouseInput[MaximumMouseButtons];

	// Any state
	bool AnyKey;
};
