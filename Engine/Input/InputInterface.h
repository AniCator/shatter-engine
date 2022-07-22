// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>

#include <Engine/Event/ActionTarget.h>
#include <Engine/Input/InputMap.h>
#include <Engine/Utility/Service/Service.h>
#include <Engine/Utility/Structures/Name.h>

struct GLFWwindow;
void InputKeyCallback( GLFWwindow* window, int KeyInput, int ScanCode, int Action, int Modifiers );
void InputCharCallback( GLFWwindow* window, unsigned int Character );
void InputMouseButtonCallback( GLFWwindow* window, int MouseButton, int Action, int Modifiers );
void InputMousePositionCallback( GLFWwindow* window, double PositionX, double PositionY );
void InputScrollCallback( GLFWwindow* window, double OffsetX, double OffsetY );
void InputJoystickStatusCallback( int Joystick, int Event );

struct FFixedPosition2D
{
	FFixedPosition2D()
	{
		X = 0;
		Y = 0;
	}

	FFixedPosition2D( const int32_t& X, const int32_t& Y )
	{
		this->X = X;
		this->Y = Y;
	}

	int32_t X;
	int32_t Y;
};

enum class EActionBindingType
{
	Keyboard = 0,
	Mouse,
	Gamepad
};

struct FActionBinding
{
	NameSymbol ActionName = NameSymbol( "unknown" );
	EActionBindingType BindingType = EActionBindingType::Keyboard;
	EKeyType BindingInput = static_cast<EKeyType>( EKey::Unknown );
	EAction BindingAction = EAction::Release;
	int BindingModifiers = 0;
	float Scale = 1.0f;
	ActionTarget TargetFunc = 0;
};

class IInput : public IEngineService
{
public:
	virtual void RegisterKeyInput( EKey KeyInput, int ScanCode, EAction Action, int Modifiers ) = 0;
	virtual void RegisterMouseButtonInput( EMouse MouseButton, EAction Action, int Modifiers ) = 0;
	virtual void RegisterMousePositionInput( double PositionX, double PositionY ) = 0;
	virtual void RegisterScrollInput( int OffsetX, int OffsetY ) = 0;
	virtual void RegisterJoystickStatus( int Joystick, int Event ) = 0;

	virtual void CreateActionBinding( const NameSymbol& ActionName ) = 0;
	virtual void AddActionBinding( const NameSymbol& ActionName, const EKey& Key, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) = 0;
	virtual void AddActionBinding( const NameSymbol& ActionName, const EMouse& Mouse, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) = 0;
	virtual void AddActionBinding( const NameSymbol& ActionName, const EGamepad& Gamepad, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) = 0;

	virtual void ClearActionBindings() = 0;

	virtual void Tick() = 0;

	virtual bool IsKeyDown( EKey KeyInput ) const = 0;
	virtual bool IsMouseDown( EMouse MouseInput ) const = 0;
	virtual bool IsAnyKeyDown() const = 0;
	virtual FFixedPosition2D GetMousePosition() const = 0;
	virtual void SetMousePosition( const FFixedPosition2D& Position ) = 0;
};


