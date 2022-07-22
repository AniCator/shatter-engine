// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Input.h"

// #include <ThirdParty/glfw-3.3.2.bin.WIN64/include/GLFW/glfw3.h>

#include <Engine/Display/Window.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Profiling/Logging.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.70/imgui.h>
#include <Engine/Display/imgui_impl_glfw.h>
#endif

#include <Engine/Input/InputMapGLFW.h>

void InputKeyCallback( GLFWwindow* window, int KeyInput, int ScanCode, int ActionInput, int Modifiers )
{
	const EKey Key = InputGLFW::CodeToKey( KeyInput );
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered() )
	{
		ImGui_ImplGlfw_KeyCallback( window, KeyInput, ScanCode, ActionInput, Modifiers );
		if( Key != EKey::NumpadSubtract )
		{
			return;
		}
	}
#endif

	if( !ImGui::GetIO().WantCaptureKeyboard )
	{
		// const EKey Key = InputGLFW::CodeToKey( KeyInput );
		const EAction Action = InputGLFW::CodeToAction( ActionInput );
		CInputLocator::Get().RegisterKeyInput( Key, ScanCode, Action, Modifiers );
	}
}

void InputCharCallback( GLFWwindow* window, unsigned int Character )
{
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyItemActive() )
	{
		ImGui_ImplGlfw_CharCallback( window, Character );
		return;
	}
#endif
}

void InputMouseButtonCallback( GLFWwindow* window, int MouseButton, int ActionInput, int Modifiers )
{
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyItemHovered() )
	{
		ImGui_ImplGlfw_MouseButtonCallback( window, MouseButton, ActionInput, Modifiers );
		return;
	}
#endif

	if( !ImGui::GetIO().WantCaptureMouse )
	{
		const EMouse Mouse = InputGLFW::CodeToMouse( MouseButton );
		const EAction Action = InputGLFW::CodeToAction( ActionInput );
		CInputLocator::Get().RegisterMouseButtonInput( Mouse, Action, Modifiers );
	}
}

void InputMousePositionCallback( GLFWwindow* window, double PositionX, double PositionY )
{
	CInputLocator::Get().RegisterMousePositionInput( PositionX, PositionY );
}

void InputScrollCallback( GLFWwindow* window, double OffsetX, double OffsetY )
{
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyWindowHovered() )
	{
		ImGui_ImplGlfw_ScrollCallback( window, OffsetX, OffsetY );
		return;
	}
#endif
	CInputLocator::Get().RegisterScrollInput( static_cast<int>( OffsetX ), static_cast<int>( OffsetY ) );
}

void InputJoystickStatusCallback( int Joystick, int Event )
{
	CInputLocator::Get().RegisterJoystickStatus( Joystick, Event );
}

CInput::CInput()
{
	AnyKey = false;

	// Initialize the Keyboard input array
	for( EKeyType KeyboardIndex = 0; KeyboardIndex < MaximumKeyboardInputs; KeyboardIndex++ )
	{
		KeyboardInput[KeyboardIndex].Input = static_cast<EKey>( KeyboardIndex );
		KeyboardInput[KeyboardIndex].Action = EAction::Unknown;
		KeyboardInput[KeyboardIndex].Modifiers = 0;
	}

	// Initialize the Mouse input array
	for( int MouseIndex = 0; MouseIndex < MaximumMouseButtons; MouseIndex++ )
	{
		MouseInput[MouseIndex].Input = MouseIndex;
		MouseInput[MouseIndex].Action = EAction::Unknown;
		MouseInput[MouseIndex].Modifiers = 0;
	}

	// Initialize the Gamepad input array
	for( int GamepadIndex = 0; GamepadIndex < MaximumJoysticks; GamepadIndex++ )
	{
		GamepadInput[GamepadIndex].Input = static_cast<EGamepad>( GamepadIndex );
		GamepadInput[GamepadIndex].Action = EAction::Unknown;
		GamepadInput[GamepadIndex].Modifiers = 0;
		GamepadInput[GamepadIndex].Scale = 1.0f;
	}
}

void CInput::RegisterKeyInput( EKey KeyInput, int ScanCode, EAction Action, int Modifiers )
{
	FKeyInput& Input = KeyboardInput[static_cast<EKeyType>( KeyInput )];
	Input.Input = KeyInput;

	// Ignore key repeat messages, toggle between press and release to latch states
	if( Input.Action == EAction::Unknown || Input.Action == EAction::Repeat )
	{
		Input.Action = Action;
	}
	else if( Input.Action == EAction::Press && Action == EAction::Release )
	{
		Input.Action = EAction::Release;
	}
	else if( Input.Action == EAction::Release && Action == EAction::Press )
	{
		Input.Action = EAction::Press;
	}

	Input.Modifiers = Modifiers;
}

void CInput::RegisterMouseButtonInput( EMouse MouseButton, EAction Action, int Modifiers )
{
	FMouseInput& Input = MouseInput[static_cast<EMouseType>( MouseButton )];

	// Latch button states
	if( Input.Action == EAction::Unknown )
	{
		Input.Action = Action;
	}
	else if( Input.Action == EAction::Press && Action == EAction::Release )
	{
		Input.Action = EAction::Release;
	}
	else if( Input.Action == EAction::Release && Action == EAction::Press )
	{
		Input.Action = EAction::Press;
	}

	Input.Modifiers = Modifiers;
}

void CInput::RegisterMousePositionInput( double PositionX, double PositionY )
{
	MousePosition.X = static_cast<int32_t>( PositionX );
	MousePosition.Y = static_cast<int32_t>( PositionY );
}

void CInput::RegisterScrollInput( int OffsetX, int OffsetY )
{
	FMouseInput& ScrollUp = MouseInput[static_cast<EMouseType>( EMouse::MouseScrollUp )];
	FMouseInput& ScrollDown = MouseInput[static_cast<EMouseType>( EMouse::MouseScrollDown )];
	if( OffsetY > 0 )
	{
		ScrollUp.Action = EAction::Release;
	}
	else if( OffsetY < 0 )
	{
		ScrollDown.Action = EAction::Release;
	}
}

void CInput::RegisterJoystickStatus( int Joystick, int Event )
{
	if( Event == GLFW_CONNECTED )
	{
		Log::Event( "Joystick %i connected.\n", Joystick );
	}
	else if( Event == GLFW_DISCONNECTED )
	{
		Log::Event( "Joystick %i disconnected.\n", Joystick );
	}

	EJoystickStatus Status = EJoystickStatus::Disconnected;
	if( Event == GLFW_CONNECTED )
	{
		Status = EJoystickStatus::Connected;
	}

	JoystickStatus[Joystick] = Status;
}

void CInput::PollJoystick( int Joystick )
{
	if( !glfwJoystickIsGamepad( Joystick ) )
	{
		JoystickStatus[Joystick] = EJoystickStatus::Disconnected;
	}

	GLFWgamepadstate GamepadState;
	glfwGetGamepadState( Joystick, &GamepadState );
	auto Name = glfwGetGamepadName( Joystick );
	for( int Index = 0; Index <= GLFW_GAMEPAD_BUTTON_LAST; Index++ )
	{
		// Check all the button states.
		const auto& State = GamepadState.buttons[Index];
		const EGamepad Gamepad = InputGLFW::CodeToGamepadButton( Index );
		const EAction Action = InputGLFW::CodeToAction( State );

		FGamepadInput& Input = GamepadInput[static_cast<EGamepadType>( Gamepad )];
		Input.Input = Gamepad;

		// Ignore key repeat messages, toggle between press and release to latch states
		if( Input.Action == EAction::Unknown && Action == EAction::Press )
		{
			Input.Action = Action;
			ActiveGamepad = Joystick;
		}
		else if( Input.Action == EAction::Press && Action == EAction::Release )
		{
			Input.Action = EAction::Release;
		}
		else if( Input.Action == EAction::Release && Action == EAction::Press )
		{
			Input.Action = EAction::Press;
		}

		Input.Modifiers = 0;
	}

	for( int Index = 0; Index <= GLFW_GAMEPAD_AXIS_LAST; Index++ )
	{
		// Check all the axis states.
		const auto& State = std::powf( GamepadState.axes[Index], 3.0f );
		const EGamepad Gamepad = InputGLFW::CodeToGamepadAxis( Index );
		EAction Action = std::fabs( State ) > 0.01f ? EAction::Press : EAction::Release;

		// Edge case for articulated gamepad triggers.
		if( Gamepad == EGamepad::GamepadLeftTrigger || Gamepad == EGamepad::GamepadRightTrigger )
		{
			Action = State > -1.0f ? EAction::Press : EAction::Release;
		}

		FGamepadInput& Input = GamepadInput[static_cast<EGamepadType>( Gamepad )];
		Input.Input = Gamepad;

		// Ignore key repeat messages, toggle between press and release to latch states
		if( Input.Action == EAction::Unknown && Action == EAction::Press )
		{
			Input.Action = Action;
		}
		else if( Input.Action == EAction::Press && Action == EAction::Release )
		{
			Input.Action = EAction::Release;
		}
		else if( Input.Action == EAction::Release && Action == EAction::Press )
		{
			Input.Action = EAction::Press;
		}

		Input.Modifiers = 0;
		Input.Scale = State;
	}
}

void CInput::AddActionBinding( const FActionBinding& Binding )
{
	CreateActionBinding( Binding.ActionName );
	const auto& Iterator = ActionBindings.find( Binding.ActionName );
	if( Iterator != ActionBindings.end() )
	{
		auto& Bindings = *Iterator;
		Bindings.second.emplace_back( Binding );
	}
}

void CInput::CreateActionBinding( const NameSymbol& ActionName )
{
	// Check if the action already exists in the binding list.
	const auto& Iterator = ActionBindings.find( ActionName );
	if( Iterator == ActionBindings.end() )
	{
		// Create a new action.
		ActionBindings.insert_or_assign( ActionName, std::vector<FActionBinding>() );
	}
}

void CInput::AddActionBinding( const NameSymbol& ActionName, const EKey& Key, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale )
{
	FActionBinding Binding;
	Binding.ActionName = ActionName;
	Binding.BindingType = EActionBindingType::Keyboard;
	Binding.BindingInput = static_cast<EKeyType>( Key );
	Binding.BindingAction = Action;
	Binding.BindingModifiers = 0;
	Binding.TargetFunc = TargetFunc;
	Binding.Scale = Scale;
	AddActionBinding( Binding );
}

void CInput::AddActionBinding( const NameSymbol& ActionName, const EMouse& Mouse, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale )
{
	FActionBinding Binding;
	Binding.ActionName = ActionName;
	Binding.BindingType = EActionBindingType::Mouse;
	Binding.BindingInput = static_cast<EKeyType>( Mouse );
	Binding.BindingAction = Action;
	Binding.BindingModifiers = 0;
	Binding.TargetFunc = TargetFunc;
	Binding.Scale = Scale;
	AddActionBinding( Binding );
}

void CInput::AddActionBinding( const NameSymbol& ActionName, const EGamepad& Gamepad, const EAction& Action, const ActionTarget& TargetFunc, const float& Scale )
{
	FActionBinding Binding;
	Binding.ActionName = ActionName;
	Binding.BindingType = EActionBindingType::Gamepad;
	Binding.BindingInput = static_cast<EKeyType>( Gamepad );
	Binding.BindingAction = Action;
	Binding.BindingModifiers = 0;
	Binding.TargetFunc = TargetFunc;
	Binding.Scale = Scale;
	AddActionBinding( Binding );
}

void CInput::ClearActionBindings()
{
	ActionBindings.clear();
}

void CInput::Tick()
{
	// Clear the 'any' key
	AnyKey = false;

	PollJoysticks();

	for( const auto& Action : ActionBindings )
	{
		for( const auto& Binding : Action.second )
		{
			bool ExecuteTargetFunction = false;
			float Scale = 1.0f;

			const auto& Keyboard = KeyboardInput[static_cast<EKeyType>( Binding.BindingInput )];
			if( Binding.BindingType == EActionBindingType::Keyboard && Keyboard.Action == Binding.BindingAction )
			{
				ExecuteTargetFunction = true;
			}

			const auto& Mouse = MouseInput[static_cast<EKeyType>( Binding.BindingInput )];
			if( Binding.BindingType == EActionBindingType::Mouse && Mouse.Action == Binding.BindingAction )
			{
				ExecuteTargetFunction = true;
			}

			const auto& Gamepad = GamepadInput[static_cast<EKeyType>( Binding.BindingInput )];
			if( Binding.BindingType == EActionBindingType::Gamepad && Gamepad.Action == Binding.BindingAction )
			{
				ExecuteTargetFunction = true;
				Scale = Gamepad.Scale;
			}

			if( ExecuteTargetFunction )
			{
				Binding.TargetFunc( Binding.Scale * Scale );
			}
		}
	}

	for( int KeyboardIndex = 0; KeyboardIndex < MaximumKeyboardInputs; KeyboardIndex++ )
	{
		FKeyInput& Input = KeyboardInput[KeyboardIndex];
		if( Input.Action == EAction::Release )
		{
			Input.Action = EAction::Unknown;
		}
		else if( Input.Action == EAction::Press )
		{
			AnyKey = true;
		}
	}

	for( int i = 0; i < MaximumMouseButtons; i++ )
	{
		FMouseInput& Input = MouseInput[i];
		if( Input.Action == EAction::Release )
		{
			Input.Action = EAction::Unknown;
		}
		else if( Input.Action == EAction::Press )
		{
			// AnyKey = true;
		}
	}

	for( int i = 1; i < static_cast<int>( EGamepad::Maximum ); i++ )
	{
		FGamepadInput& Input = GamepadInput[i];
		if( Input.Action == EAction::Release )
		{
			Input.Action = EAction::Unknown;
		}
		else if( Input.Action == EAction::Press )
		{
			AnyKey = true;
		}
	}
}

void CInput::PollJoysticks()
{
	for( int Index = 0; Index < MaximumJoysticks; Index++ )
	{
		if( JoystickStatus[Index] == EJoystickStatus::Connected )
		{
			PollJoystick( Index );
			break;
		}
	}
}

bool CInput::IsKeyDown( EKey KeyInput ) const
{
	return KeyboardInput[static_cast<EKeyType>( KeyInput )].Action == EAction::Press;
}

bool CInput::IsMouseDown( EMouse Input ) const
{
	return MouseInput[static_cast<EMouseType>( Input )].Action == EAction::Press;
}

bool CInput::IsAnyKeyDown() const
{
	return AnyKey;
}

FFixedPosition2D CInput::GetMousePosition() const
{
	return MousePosition;
}

void CInput::SetMousePosition( const FFixedPosition2D& Position )
{
	MousePosition = Position;
	glfwSetCursorPos( CWindow::Get().Handle(), MousePosition.X, MousePosition.Y );
}

const FKeyInput* CInput::GetKeys() const
{
	return KeyboardInput;
}

const FGamepadInput* CInput::GetGamepad() const
{
	return GamepadInput;
}

const FMouseInput* CInput::GetMouse() const
{
	return MouseInput;
}
