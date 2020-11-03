// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Input/InputInterface.h>

#include <vector>

static const constexpr EKeyType MaximumKeyboardInputs = static_cast<EKeyType>( EKey::Maximum );
static const constexpr int MaximumJoysticks = 16;
static const constexpr EMouseType MaximumMouseButtons = static_cast<EMouseType>( EMouse::Maximum );
static const constexpr EGamepadType MaximumGamepadButtons = static_cast<EGamepadType>( EGamepad::Maximum );

struct FKeyInput
{
	EKey Input = EKey::Unknown;
	EAction Action = EAction::Unknown;
	int Modifiers = 0;
};

struct FMouseInput
{
	uint32_t Input = 0;
	EAction Action = EAction::Unknown;
	int Modifiers = 0;
};

struct FGamepadInput
{
	EGamepad Input = EGamepad::Unknown;
	EAction Action = EAction::Unknown;
	int Modifiers = 0;
	float Scale = 1.0f;
};

enum class EJoystickStatus : uint8_t
{
	Disconnected,
	Connected
};

class CInput : public IInput
{
public:
	CInput();

	CInput( CInput const& ) = delete;
	void operator=( CInput const& ) = delete;

	virtual void RegisterKeyInput( EKey KeyInput, int ScanCode, EAction Action, int Modifiers ) override;
	virtual void RegisterMouseButtonInput( EMouse MouseButton, EAction Action, int Modifiers ) override;
	virtual void RegisterMousePositionInput( double PositionX, double PositionY ) override;
	virtual void RegisterScrollInput( int OffsetX, int OffsetY ) override;
	virtual void RegisterJoystickStatus( int Joystick, int Event ) override;

	virtual void CreateActionBinding( const FName& ActionName ) override;
	virtual void AddActionBinding( const FName& ActionName, const EKey& Key, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	virtual void AddActionBinding( const FName& ActionName, const EMouse& Mouse, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	virtual void AddActionBinding( const FName& ActionName, const EGamepad& Gamepad, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	virtual void ClearActionBindings() override;

	virtual void Tick() override;
	void PollJoysticks();

	virtual bool IsKeyDown( EKey KeyInput ) const override;
	virtual bool IsAnyKeyDown() const override;
	virtual FFixedPosition2D GetMousePosition() const override;
	virtual void SetMousePosition( const FFixedPosition2D& Position ) override;

private:
	void PollJoystick( int Joystick );
	void AddActionBinding( const FActionBinding& Binding );
	std::map<FName, std::vector<FActionBinding>> ActionBindings;

	// Keyboard Inputs
	FKeyInput KeyboardInput[MaximumKeyboardInputs] = {};

	// Joystick status
	EJoystickStatus JoystickStatus[MaximumJoysticks] = {};
	FGamepadInput GamepadInput[MaximumGamepadButtons] = {};
	int ActiveGamepad = -1;

	// Mouse button states
	FMouseInput MouseInput[MaximumMouseButtons] = {};

	FFixedPosition2D MousePosition;

	// Any state
	bool AnyKey;
};
