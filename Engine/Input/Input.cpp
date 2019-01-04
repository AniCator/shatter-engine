// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Input.h"

#include <GLFW/glfw3.h>

#include <Engine/Display/Window.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Profiling/Logging.h>

#ifdef IMGUI_ENABLED
#include <ThirdParty/imgui-1.52/imgui.h>
#include <Engine/Display/imgui_impl_glfw_gl3.h>
#endif

void InputKeyCallback( GLFWwindow* window, int KeyInput, int ScanCode, int Action, int Modifiers )
{
#ifdef IMGUI_ENABLED
	if( ImGui::IsAnyItemHovered() )
	{
		ImGui_ImplGlfwGL3_KeyCallback( window, KeyInput, ScanCode, Action, Modifiers );
		return;
	}
#endif
	CInputLocator::GetService().RegisterKeyInput( KeyInput, ScanCode, Action, Modifiers );
}

void InputCharCallback( GLFWwindow* window, unsigned int Character )
{
#ifdef IMGUI_ENABLED
	if( ImGui::IsAnyItemHovered() )
	{
		ImGui_ImplGlfwGL3_CharCallback( window, Character );
		return;
	}
#endif
}

void InputMouseButtonCallback( GLFWwindow* window, int MouseButton, int Action, int Modifiers )
{
#ifdef IMGUI_ENABLED
	if( ImGui::IsAnyItemHovered() )
	{
		ImGui_ImplGlfwGL3_MouseButtonCallback( window, MouseButton, Action, Modifiers );
		return;
	}
#endif
	CInputLocator::GetService().RegisterMouseButtonInput( MouseButton, Action, Modifiers );
}

void InputMousePositionCallback( GLFWwindow* window, double PositionX, double PositionY )
{
	CInputLocator::GetService().RegisterMousePositionInput( PositionX, PositionY );
}

void InputScrollCallback( GLFWwindow* window, double OffsetX, double OffsetY )
{
#ifdef IMGUI_ENABLED
	if( ImGui::IsAnyItemActive() )
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
	for( int KeyboardIndex = 0; KeyboardIndex < MaximumKeyboardInputs; KeyboardIndex++ )
	{
		KeyboardInput[KeyboardIndex].Input = KeyboardIndex;
		KeyboardInput[KeyboardIndex].Action = -1;
		KeyboardInput[KeyboardIndex].Modifiers = 0;
	}

	// Initialize the Mouse input array
	for( int MouseIndex = 0; MouseIndex < MaximumMouseButtons; MouseIndex++ )
	{
		MouseInput[MouseIndex].Input = MouseIndex;
		MouseInput[MouseIndex].Action = -1;
		MouseInput[MouseIndex].Modifiers = 0;
	}
}

void CInput::RegisterKeyInput( int KeyInput, int ScanCode, int Action, int Modifiers )
{
	FInput& Input = KeyboardInput[KeyInput];
	Input.Input = KeyInput;

	// Ignore key repeat messages, toggle between press and release to latch states
	if( Input.Action == -1 )
	{
		Input.Action = Action;
	}
	else if( Input.Action == GLFW_PRESS && Action == GLFW_RELEASE )
	{
		Input.Action = GLFW_RELEASE;
	}
	else if( Input.Action == GLFW_RELEASE && Action == GLFW_PRESS )
	{
		Input.Action = GLFW_PRESS;
	}

	Input.Modifiers = Modifiers;
}

void CInput::RegisterMouseButtonInput( int MouseButton, int Action, int Modifiers )
{
	FInput& Input = MouseInput[MouseButton];

	// Latch button states
	if( Input.Action == -1 )
	{
		Input.Action = Action;
	}
	else if( Input.Action == GLFW_PRESS && Action == GLFW_RELEASE )
	{
		Input.Action = GLFW_RELEASE;
	}
	else if( Input.Action == GLFW_RELEASE && Action == GLFW_PRESS )
	{
		Input.Action = GLFW_PRESS;
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

void CInput::AddActionBinding( EActionBindingType BindingType, int KeyInput, int Action, ActionTarget TargetFunc )
{
	FActionBinding ActionBinding;
	ActionBinding.BindingType = BindingType;
	ActionBinding.BindingInput = KeyInput;
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

		if( ActionBinding.BindingType == EActionBindingType::Keyboard && KeyboardInput[ActionBinding.BindingInput].Action == ActionBinding.BindingAction )
		{
			ExecuteTargetFunction = true;
		}

		if( ActionBinding.BindingType == EActionBindingType::Mouse && MouseInput[ActionBinding.BindingInput].Action == ActionBinding.BindingAction )
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
		FInput& Input = KeyboardInput[KeyboardIndex];
		if( Input.Action == GLFW_RELEASE )
		{
			Input.Action = -1;
		}
		else if( Input.Action == GLFW_PRESS )
		{
			AnyKey = true;
		}
	}

	for( int i = 0; i < MaximumMouseButtons; i++ )
	{
		FInput& Input = MouseInput[i];
		if( Input.Action == GLFW_RELEASE )
		{
			Input.Action = -1;
		}
	}
}

bool CInput::IsKeyDown( int KeyInput ) const
{
	return KeyboardInput[KeyInput].Action == GLFW_PRESS;
}

bool CInput::IsAnyKeyDown() const
{
	return AnyKey;
}

FFixedPosition2D CInput::GetMousePosition() const
{
	return MousePosition;
}
