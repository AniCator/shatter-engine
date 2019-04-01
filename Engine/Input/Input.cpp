// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Input.h"

#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h>

#include <Engine/Display/Window.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Profiling/Logging.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.52/imgui.h>
#include <Engine/Display/imgui_impl_glfw_gl3.h>
#endif

#include <Engine/Input/InputMapGLFW.h>

void InputKeyCallback( GLFWwindow* window, int KeyInput, int ScanCode, int ActionInput, int Modifiers )
{
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyItemActive() )
	{
		ImGui_ImplGlfwGL3_KeyCallback( window, KeyInput, ScanCode, ActionInput, Modifiers );
		return;
	}
#endif

	const EKey Key = InputGLFW::CodeToKey( KeyInput );
	const EAction Action = InputGLFW::CodeToAction( ActionInput );
	CInputLocator::GetService().RegisterKeyInput( Key, ScanCode, Action, Modifiers );
}

void InputCharCallback( GLFWwindow* window, unsigned int Character )
{
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyItemActive() )
	{
		ImGui_ImplGlfwGL3_CharCallback( window, Character );
		return;
	}
#endif
}

void InputMouseButtonCallback( GLFWwindow* window, int MouseButton, int ActionInput, int Modifiers )
{
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyItemHovered() )
	{
		ImGui_ImplGlfwGL3_MouseButtonCallback( window, MouseButton, ActionInput, Modifiers );
		return;
	}
#endif

	const EMouse Mouse = InputGLFW::CodeToMouse( MouseButton );
	const EAction Action = InputGLFW::CodeToAction( ActionInput );
	CInputLocator::GetService().RegisterMouseButtonInput( Mouse, Action, Modifiers );
}

void InputMousePositionCallback( GLFWwindow* window, double PositionX, double PositionY )
{
	CInputLocator::GetService().RegisterMousePositionInput( PositionX, PositionY );
}

void InputScrollCallback( GLFWwindow* window, double OffsetX, double OffsetY )
{
#if defined( IMGUI_ENABLED )
	if( ImGui::IsAnyWindowHovered() )
	{
		ImGui_ImplGlfwGL3_ScrollCallback( window, OffsetX, OffsetY );
		return;
	}
#endif
	CInputLocator::GetService().RegisterScrollInput( static_cast<int>( OffsetX ), static_cast<int>( OffsetY ) );
}

void InputJoystickStatusCallback( int Joystick, int Event )
{
	CInputLocator::GetService().RegisterJoystickStatus( Joystick, Event );
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
}

void CInput::RegisterKeyInput( EKey KeyInput, int ScanCode, EAction Action, int Modifiers )
{
	FKeyInput& Input = KeyboardInput[static_cast<EKeyType>( KeyInput )];
	Input.Input = KeyInput;

	// Ignore key repeat messages, toggle between press and release to latch states
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
}

void CInput::AddActionBinding( FActionBinding ActionBinding )
{
	ActionBindings.push_back( ActionBinding );
}

void CInput::AddActionBinding( EKey KeyInput, EAction Action, ActionTarget TargetFunc )
{
	FActionBinding ActionBinding;
	ActionBinding.BindingType = EActionBindingType::Keyboard;
	ActionBinding.BindingInput = static_cast<EKeyType>( KeyInput );
	ActionBinding.BindingAction = Action;
	ActionBinding.TargetFunc = TargetFunc;
	AddActionBinding( ActionBinding );
}

void CInput::AddActionBinding( EMouse KeyInput, EAction Action, ActionTarget TargetFunc )
{
	FActionBinding ActionBinding;
	ActionBinding.BindingType = EActionBindingType::Mouse;
	ActionBinding.BindingInput = static_cast<EKeyType>( KeyInput );
	ActionBinding.BindingAction = Action;
	ActionBinding.TargetFunc = TargetFunc;
	AddActionBinding( ActionBinding );
}

void CInput::AddActionBinding( EGamepad KeyInput, EAction Action, ActionTarget TargetFunc )
{
	FActionBinding ActionBinding;
	ActionBinding.BindingType = EActionBindingType::Gamepad;
	ActionBinding.BindingInput = static_cast<EKeyType>( KeyInput );
	ActionBinding.BindingAction = Action;
	ActionBinding.TargetFunc = TargetFunc;
	AddActionBinding( ActionBinding );
}

void CInput::ClearActionBindings()
{
	ActionBindings.clear();
}

void CInput::Tick()
{
	// Clear the 'any' key
	AnyKey = false;

	for( auto ActionBinding : ActionBindings )
	{
		bool ExecuteTargetFunction = false;

		if( ActionBinding.BindingType == EActionBindingType::Keyboard && KeyboardInput[static_cast<EKeyType>( ActionBinding.BindingInput )].Action == ActionBinding.BindingAction )
		{
			ExecuteTargetFunction = true;
		}

		if( ActionBinding.BindingType == EActionBindingType::Mouse && MouseInput[static_cast<EKeyType>( ActionBinding.BindingInput )].Action == ActionBinding.BindingAction )
		{
			ExecuteTargetFunction = true;
		}

		if( ExecuteTargetFunction )
		{
			ActionBinding.TargetFunc();
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
	}
}

bool CInput::IsKeyDown( int KeyInput ) const
{
	return KeyboardInput[KeyInput].Action == EAction::Press;
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
