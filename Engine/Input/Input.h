// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Input/InputInterface.h>

#include <vector>
#include <map>

static constexpr EKeyType MaximumKeyboardInputs = static_cast<EKeyType>( EKey::Maximum );
static constexpr int MaximumJoysticks = 16;
static constexpr EMouseType MaximumMouseButtons = static_cast<EMouseType>( EMouse::Maximum );
static constexpr EGamepadType MaximumGamepadButtons = static_cast<EGamepadType>( EGamepad::Maximum );

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

const char* GetMouseLabel( const EMouseType Index );
const char* GetGamepadLabel( const EGamepadType Index );

using ActionMap = std::map<NameSymbol, std::vector<FActionBinding>>;
class CInput : public IInput
{
public:
	CInput();

	CInput( CInput const& ) = delete;
	void operator=( CInput const& ) = delete;

	void RegisterKeyInput( EKey KeyInput, int ScanCode, EAction Action, int Modifiers ) override;
	void RegisterMouseButtonInput( EMouse MouseButton, EAction Action, int Modifiers ) override;
	void RegisterMousePositionInput( double PositionX, double PositionY ) override;
	void RegisterScrollInput( int OffsetX, int OffsetY ) override;
	void RegisterJoystickStatus( int Joystick, int Event ) override;

	void CreateActionBinding( const NameSymbol& ActionName ) override;
	void AddActionBinding( const NameSymbol& ActionName, const EKey& Key, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	void AddActionBinding( const NameSymbol& ActionName, const EMouse& Mouse, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	void AddActionBinding( const NameSymbol& ActionName, const EGamepad& Gamepad, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale = 1.0f ) override;
	void ClearActionBindings() override;

	void Tick() override;
	void PollJoysticks();

	bool IsKeyDown( EKey KeyInput ) const override;
	bool IsMouseDown( EMouse MouseInput ) const override;
	bool IsAnyKeyDown() const override;
	FFixedPosition2D GetMousePosition() const override;
	void SetMousePosition( const FFixedPosition2D& Position ) override;

	const FKeyInput* GetKeys() const;
	const FGamepadInput* GetGamepad() const;
	const FMouseInput* GetMouse() const;

	const ActionMap& GetBindings() const;
private:
	void PollJoystick( int Joystick );
	void ClearJoystick( int Joystick );
	void AddActionBinding( const FActionBinding& Binding );
	ActionMap ActionBindings;

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
