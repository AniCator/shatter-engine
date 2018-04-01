// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Event/ActionTarget.h>

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

enum class EActionBindingType
{
	Keyboard = 0,
	Mouse,
	Joystick
};

struct FActionBinding
{
	FActionBinding()
	{
		BindingType = EActionBindingType::Keyboard;
		BindingInput = -1;
		BindingAction = 1;
		BindingModifiers = 0;
		TargetFunc = 0;
	}

	EActionBindingType BindingType;
	int BindingInput;
	int BindingAction;
	int BindingModifiers;
	ActionTarget TargetFunc;
};

struct GLFWwindow;
void InputKeyCallback( GLFWwindow* window, int KeyInput, int ScanCode, int Action, int Modifiers );
void InputCharCallback( GLFWwindow* window, unsigned int Character );
void InputMouseButtonCallback( GLFWwindow* window, int MouseButton, int Action, int Modifiers );
void InputScrollCallback( GLFWwindow* window, double OffsetX, double OffsetY );
void InputJoystickStatusCallback( int Joystick, int Event );

class CInput
{
public:
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

public:
	static CInput& GetInstance()
	{
		static CInput StaticInstance;
		return StaticInstance;
	}
private:
	CInput();

	CInput( CInput const& ) = delete;
	void operator=( CInput const& ) = delete;

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