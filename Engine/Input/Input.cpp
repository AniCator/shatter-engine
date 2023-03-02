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

void UpdateImGuiNavigation( FGamepadInput* Inputs, EJoystickStatus Status )
{
	ImGuiIO& io = ImGui::GetIO();

	// Set the navigation inputs to 0.
	memset( io.NavInputs, 0, sizeof( io.NavInputs ) );
	if( ( io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad ) == 0 )
		return; // Don't do anything if gamepad navigation is disabled.

	static std::vector<std::pair<ImGuiNavInput_, EGamepad>> BinaryInputs = {
		{ ImGuiNavInput_Activate,	EGamepad::GamepadFaceButtonDown },			// Cross / A
		{ ImGuiNavInput_Cancel,		EGamepad::GamepadFaceButtonRight },			// Circle / B
		{ ImGuiNavInput_Menu,		EGamepad::GamepadFaceButtonLeft },			// Square / X
		{ ImGuiNavInput_Input,		EGamepad::GamepadFaceButtonUp },			// Triangle / Y
		{ ImGuiNavInput_DpadLeft,	EGamepad::GamepadDirectionalButtonLeft },	// D-Pad Left
		{ ImGuiNavInput_DpadRight,	EGamepad::GamepadDirectionalButtonRight },	// D-Pad Right
		{ ImGuiNavInput_DpadUp,		EGamepad::GamepadDirectionalButtonUp },		// D-Pad Up
		{ ImGuiNavInput_DpadDown,	EGamepad::GamepadDirectionalButtonDown },	// D-Pad Down
		{ ImGuiNavInput_FocusPrev,	EGamepad::GamepadLeftShoulder },			// L1 / LB
		{ ImGuiNavInput_FocusNext,	EGamepad::GamepadRightShoulder },			// R1 / RB
		{ ImGuiNavInput_TweakSlow,	EGamepad::GamepadLeftShoulder },			// L1 / LB
		{ ImGuiNavInput_TweakFast,	EGamepad::GamepadRightShoulder }			// R1 / RB
	};

	for( const auto& Input : BinaryInputs )
	{
		const auto Value = static_cast<size_t>( Input.second );
		if( Inputs[Value].Action == EAction::Release && io.NavInputs[Input.first] == 0.0f )
		{
			io.NavInputs[Input.first] = 1.0f;
			Log::Event( "Release\n" );
		}
		else if( io.NavInputs[Input.first] > 0.0f )
		{
			io.NavInputs[Input.first] = 0.0f;
			Log::Event( "Unknown\n" );
		}
	}

	// TODO: Handle analog inputs for ImGui.
	/*std::vector<std::pair<int, EGamepad>> AnalogInputs = {
	{ ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f },
	{ ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f },
	{ ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f },
	{ ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f }
	};*/

	if( Status == EJoystickStatus::Connected )
	{
		io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	}
	else
	{
		io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
	}
}

const char* MouseLabels[] =
{
	"Unknown",

	"MouseX",
	"MouseY",

	"MouseScrollUp",
	"MouseScrollDown",

	"LeftMouseButton",
	"RightMouseButton",
	"MiddleMouseButton",

	"MouseButton4",
	"MouseButton5",
	"MouseButton6",
	"MouseButton7",
	"MouseButton8",

	"Maximum"
};

const char* GetMouseLabel( const EMouseType Index )
{
	return MouseLabels[Index];
}

const char* GamepadLabels[] =
{
	"Unknown",

	"GamepadFaceButtonUp",
	"GamepadFaceButtonDown",
	"GamepadFaceButtonLeft",
	"GamepadFaceButtonRight",

	"GamepadDirectionalButtonUp",
	"GamepadDirectionalButtonDown",
	"GamepadDirectionalButtonLeft",
	"GamepadDirectionalButtonRight",

	"GamepadLeftStickX",
	"GamepadLeftStickY",
	"GamepadLeftStickTrigger",

	"GamepadLeftTrigger",
	"GamepadLeftShoulder",
	"GamepadLeftSpecial",

	"GamepadRightStickX",
	"GamepadRightStickY",
	"GamepadRightStickTrigger",

	"GamepadRightTrigger",
	"GamepadRightShoulder",
	"GamepadRightSpecial",

	"Maximum"
};

const char* GetGamepadLabel( const EGamepadType Index )
{
	return GamepadLabels[Index];
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
	EJoystickStatus Status = EJoystickStatus::Disconnected;
	if( Event == GLFW_CONNECTED )
	{
		Status = EJoystickStatus::Connected;
	}

	const auto IsGamepad = glfwJoystickIsGamepad( Joystick );
	if( Status == EJoystickStatus::Connected )
	{
		Log::Event( "%s %i connected.\n", IsGamepad ? "Gamepad" : "Joystick", Joystick );
	}
	else if( Status == EJoystickStatus::Disconnected )
	{
		Log::Event( "%s %i disconnected.\n", IsGamepad ? "Gamepad" : "Joystick", Joystick );
	}

	if( JoystickStatus[Joystick] == EJoystickStatus::Connected && Status == EJoystickStatus::Disconnected )
	{
		// Joystick status changed, clear the inputs.
		ClearJoystick( Joystick );
	}

	JoystickStatus[Joystick] = Status;
}

const ActionMap& CInput::GetBindings() const
{
	return ActionBindings;
}

void CInput::PollJoystick( int Joystick )
{
	if( !glfwJoystickIsGamepad( Joystick ) )
	{
		JoystickStatus[Joystick] = EJoystickStatus::Disconnected;
		ClearJoystick( Joystick );
		return;
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

void CInput::ClearJoystick( int Joystick )
{
	for( int i = 1; i < static_cast<int>( EGamepad::Maximum ); i++ )
	{
		// Reset all actions.
		FGamepadInput& Input = GamepadInput[i];
		Input.Action = EAction::Unknown;
	}
}

void CInput::AddActionBinding( const FActionBinding& Binding )
{
	CreateActionBinding( Binding.ActionName );
	const auto& Iterator = ActionBindings.find( Binding.ActionName );
	if( Iterator == ActionBindings.end() )
		return;
	
	auto& Bindings = *Iterator;
	Bindings.second.emplace_back( Binding );
}

void CInput::CreateActionBinding( const NameSymbol& ActionName )
{
	// Check if the action already exists in the binding list.
	const auto& Iterator = ActionBindings.find( ActionName );
	if( Iterator != ActionBindings.end() )
		return;

	// Create a new action.
	ActionBindings.insert_or_assign( ActionName, std::vector<FActionBinding>() );
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
