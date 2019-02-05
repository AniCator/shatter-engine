// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>

#include <Engine/Event/ActionTarget.h>
#include <Engine/Input/InputMap.h>
#include <Engine/Utility/Service/Service.h>

struct GLFWwindow;
void InputKeyCallback( GLFWwindow* window, int KeyInput, int ScanCode, int Action, int Modifiers );
void InputCharCallback( GLFWwindow* window, unsigned int Character );
void InputMouseButtonCallback( GLFWwindow* window, int MouseButton, int Action, int Modifiers );
void InputMousePositionCallback( GLFWwindow* window, double PositionX, double PositionY );
void InputScrollCallback( GLFWwindow* window, double OffsetX, double OffsetY );
void InputJoystickStatusCallback( int Joystick, int Event );

struct FFixedPosition2D
{
	int32_t X;
	int32_t Y;
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
		BindingInput = EKey::Unknown;
		BindingAction = 1;
		BindingModifiers = 0;
		TargetFunc = 0;
	}

	EActionBindingType BindingType;
	EKey BindingInput;
	int BindingAction;
	int BindingModifiers;
	ActionTarget TargetFunc;
};

class IInput : public IEngineService
{
public:
	virtual void RegisterKeyInput( EKey KeyInput, int ScanCode, int Action, int Modifiers ) = 0;
	virtual void RegisterMouseButtonInput( int MouseButton, int Action, int Modifiers ) = 0;
	virtual void RegisterMousePositionInput( double PositionX, double PositionY ) = 0;
	virtual void RegisterScrollInput( int OffsetX, int OffsetY ) = 0;
	virtual void RegisterJoystickStatus( int Joystick, int Event ) = 0;

	virtual void AddActionBinding( FActionBinding ActionBinding ) = 0;
	virtual void AddActionBinding( EActionBindingType BindingType, EKey KeyInput, int Action, ActionTarget TargetFunc ) = 0;
	virtual void ClearActionBindings() = 0;

	virtual void Tick() = 0;

	virtual bool IsKeyDown( int KeyInput ) const = 0;
	virtual bool IsAnyKeyDown() const = 0;
	virtual FFixedPosition2D GetMousePosition() const = 0;
};


