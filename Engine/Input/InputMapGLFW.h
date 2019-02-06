// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "InputMap.h"
#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h>

struct FEKeyPairGLFW
{
	EKey Enum;
	int Key;
};

namespace InputGLFW
{
	static const int ActionToCode[EAction::Maximum]
	{
		GLFW_DONT_CARE,

		GLFW_RELEASE,
		GLFW_PRESS,
		GLFW_REPEAT
	};

	static EAction CodeToAction( int Action )
	{
		switch( Action )
		{
			case GLFW_RELEASE:				return EAction::Release;
			case GLFW_PRESS:				return EAction::Press;
			case GLFW_REPEAT:				return EAction::Repeat;
			default:						return EAction::Unknown;
		}
	}

	static const int KeyToCode[EKey::Maximum]
	{
		GLFW_KEY_UNKNOWN,
		GLFW_KEY_SPACE,
		GLFW_KEY_APOSTROPHE,
		GLFW_KEY_COMMA,
		GLFW_KEY_MINUS,
		GLFW_KEY_PERIOD,
		GLFW_KEY_SLASH,

		GLFW_KEY_0,
		GLFW_KEY_1,
		GLFW_KEY_2,
		GLFW_KEY_3,
		GLFW_KEY_4,
		GLFW_KEY_5,
		GLFW_KEY_6,
		GLFW_KEY_7,
		GLFW_KEY_8,
		GLFW_KEY_9,

		GLFW_KEY_SEMICOLON,
		GLFW_KEY_EQUAL,
		GLFW_KEY_A,
		GLFW_KEY_B,
		GLFW_KEY_C,
		GLFW_KEY_D,
		GLFW_KEY_E,
		GLFW_KEY_F,
		GLFW_KEY_G,
		GLFW_KEY_H,
		GLFW_KEY_I,
		GLFW_KEY_J,
		GLFW_KEY_K,
		GLFW_KEY_L,
		GLFW_KEY_M,
		GLFW_KEY_N,
		GLFW_KEY_O,
		GLFW_KEY_P,
		GLFW_KEY_Q,
		GLFW_KEY_R,
		GLFW_KEY_S,
		GLFW_KEY_T,
		GLFW_KEY_U,
		GLFW_KEY_V,
		GLFW_KEY_W,
		GLFW_KEY_X,
		GLFW_KEY_Y,
		GLFW_KEY_Z,
		GLFW_KEY_LEFT_BRACKET,
		GLFW_KEY_BACKSLASH,
		GLFW_KEY_RIGHT_BRACKET,
		GLFW_KEY_GRAVE_ACCENT,
		GLFW_KEY_WORLD_1, // Non-US Key 1
		GLFW_KEY_WORLD_2, // Non-US Key 2

		GLFW_KEY_ESCAPE,
		GLFW_KEY_ENTER,
		GLFW_KEY_TAB,
		GLFW_KEY_BACKSPACE,
		GLFW_KEY_INSERT,
		GLFW_KEY_DELETE,
		GLFW_KEY_RIGHT,
		GLFW_KEY_LEFT,
		GLFW_KEY_DOWN,
		GLFW_KEY_UP,
		GLFW_KEY_PAGE_UP,
		GLFW_KEY_PAGE_DOWN,
		GLFW_KEY_HOME,
		GLFW_KEY_END,
		GLFW_KEY_CAPS_LOCK,
		GLFW_KEY_SCROLL_LOCK,
		GLFW_KEY_NUM_LOCK,
		GLFW_KEY_PRINT_SCREEN,
		GLFW_KEY_PAUSE,

		GLFW_KEY_F1,
		GLFW_KEY_F2,
		GLFW_KEY_F3,
		GLFW_KEY_F4,
		GLFW_KEY_F5,
		GLFW_KEY_F6,
		GLFW_KEY_F7,
		GLFW_KEY_F8,
		GLFW_KEY_F9,
		GLFW_KEY_F10,
		GLFW_KEY_F11,
		GLFW_KEY_F12,
		GLFW_KEY_F13,
		GLFW_KEY_F14,
		GLFW_KEY_F15,
		GLFW_KEY_F16,
		GLFW_KEY_F17,
		GLFW_KEY_F18,
		GLFW_KEY_F19,
		GLFW_KEY_F20,
		GLFW_KEY_F21,
		GLFW_KEY_F22,
		GLFW_KEY_F23,
		GLFW_KEY_F24,
		GLFW_KEY_F25,

		GLFW_KEY_KP_0,
		GLFW_KEY_KP_1,
		GLFW_KEY_KP_2,
		GLFW_KEY_KP_3,
		GLFW_KEY_KP_4,
		GLFW_KEY_KP_5,
		GLFW_KEY_KP_6,
		GLFW_KEY_KP_7,
		GLFW_KEY_KP_8,
		GLFW_KEY_KP_9,
		GLFW_KEY_KP_DECIMAL,
		GLFW_KEY_KP_DIVIDE,
		GLFW_KEY_KP_MULTIPLY,
		GLFW_KEY_KP_SUBTRACT,
		GLFW_KEY_KP_ADD,
		GLFW_KEY_KP_ENTER,
		GLFW_KEY_KP_EQUAL,

		GLFW_KEY_LEFT_SHIFT,
		GLFW_KEY_LEFT_CONTROL,
		GLFW_KEY_LEFT_ALT,
		GLFW_KEY_LEFT_SUPER, // Command/Windows key
		GLFW_KEY_RIGHT_SHIFT,
		GLFW_KEY_RIGHT_CONTROL,
		GLFW_KEY_RIGHT_ALT,
		GLFW_KEY_RIGHT_SUPER, // Command/Windows key
		GLFW_KEY_MENU // List menu key
	};

	static EKey CodeToKey( int Key )
	{
		switch( Key )
		{
			case GLFW_KEY_SPACE:			return EKey::Space;
			case GLFW_KEY_APOSTROPHE:		return EKey::Apostrophe;
			case GLFW_KEY_COMMA:			return EKey::Comma;
			case GLFW_KEY_MINUS:			return EKey::Minus;
			case GLFW_KEY_PERIOD:			return EKey::Period;
			case GLFW_KEY_SLASH:			return EKey::Slash;

			case GLFW_KEY_0:				return EKey::Number0;
			case GLFW_KEY_1:				return EKey::Number1;
			case GLFW_KEY_2:				return EKey::Number2;
			case GLFW_KEY_3:				return EKey::Number3;
			case GLFW_KEY_4:				return EKey::Number4;
			case GLFW_KEY_5:				return EKey::Number5;
			case GLFW_KEY_6:				return EKey::Number6;
			case GLFW_KEY_7:				return EKey::Number7;
			case GLFW_KEY_8:				return EKey::Number8;
			case GLFW_KEY_9:				return EKey::Number9;

			case GLFW_KEY_SEMICOLON:		return EKey::Semicolon;
			case GLFW_KEY_EQUAL:			return EKey::Equal;
			case GLFW_KEY_A:				return EKey::A;
			case GLFW_KEY_B:				return EKey::B;
			case GLFW_KEY_C:				return EKey::C;
			case GLFW_KEY_D:				return EKey::D;
			case GLFW_KEY_E:				return EKey::E;
			case GLFW_KEY_F:				return EKey::F;
			case GLFW_KEY_G:				return EKey::G;
			case GLFW_KEY_H:				return EKey::H;
			case GLFW_KEY_I:				return EKey::I;
			case GLFW_KEY_J:				return EKey::J;
			case GLFW_KEY_K:				return EKey::K;
			case GLFW_KEY_L:				return EKey::L;
			case GLFW_KEY_M:				return EKey::M;
			case GLFW_KEY_N:				return EKey::N;
			case GLFW_KEY_O:				return EKey::O;
			case GLFW_KEY_P:				return EKey::P;
			case GLFW_KEY_Q:				return EKey::Q;
			case GLFW_KEY_R:				return EKey::R;
			case GLFW_KEY_S:				return EKey::S;
			case GLFW_KEY_T:				return EKey::T;
			case GLFW_KEY_U:				return EKey::U;
			case GLFW_KEY_V:				return EKey::V;
			case GLFW_KEY_W:				return EKey::W;
			case GLFW_KEY_X:				return EKey::X;
			case GLFW_KEY_Y:				return EKey::Y;
			case GLFW_KEY_Z:				return EKey::Z;
			case GLFW_KEY_LEFT_BRACKET:		return EKey::LeftBracket;
			case GLFW_KEY_BACKSLASH:		return EKey::Backslash;
			case GLFW_KEY_RIGHT_BRACKET:	return EKey::RightBracket;
			case GLFW_KEY_GRAVE_ACCENT:		return EKey::GraveAccent;
			case GLFW_KEY_WORLD_1:			return EKey::World1; // Non-US Key 1
			case GLFW_KEY_WORLD_2:			return EKey::World2;  // Non-US Key 2

			case GLFW_KEY_ESCAPE:			return EKey::Escape;
			case GLFW_KEY_ENTER:			return EKey::Enter;
			case GLFW_KEY_TAB:				return EKey::Tab;
			case GLFW_KEY_BACKSPACE:		return EKey::Backspace;
			case GLFW_KEY_INSERT:			return EKey::Insert;
			case GLFW_KEY_DELETE:			return EKey::Delete;
			case GLFW_KEY_RIGHT:			return EKey::Right;
			case GLFW_KEY_LEFT:				return EKey::Left;
			case GLFW_KEY_DOWN:				return EKey::Down;
			case GLFW_KEY_UP:				return EKey::Up;
			case GLFW_KEY_PAGE_UP:			return EKey::PageUp;
			case GLFW_KEY_PAGE_DOWN:		return EKey::PageDown;
			case GLFW_KEY_HOME:				return EKey::Home;
			case GLFW_KEY_END:				return EKey::End;
			case GLFW_KEY_CAPS_LOCK:		return EKey::CapsLock;
			case GLFW_KEY_SCROLL_LOCK:		return EKey::ScrollLock;
			case GLFW_KEY_NUM_LOCK:			return EKey::NumLock;
			case GLFW_KEY_PRINT_SCREEN:		return EKey::PrintScreen;
			case GLFW_KEY_PAUSE:			return EKey::Pause;

			case GLFW_KEY_F1:				return EKey::F1;
			case GLFW_KEY_F2:				return EKey::F2;
			case GLFW_KEY_F3:				return EKey::F3;
			case GLFW_KEY_F4:				return EKey::F4;
			case GLFW_KEY_F5:				return EKey::F5;
			case GLFW_KEY_F6:				return EKey::F6;
			case GLFW_KEY_F7:				return EKey::F7;
			case GLFW_KEY_F8:				return EKey::F8;
			case GLFW_KEY_F9:				return EKey::F9;
			case GLFW_KEY_F10:				return EKey::F10;
			case GLFW_KEY_F11:				return EKey::F11;
			case GLFW_KEY_F12:				return EKey::F12;
			case GLFW_KEY_F13:				return EKey::F13;
			case GLFW_KEY_F14:				return EKey::F14;
			case GLFW_KEY_F15:				return EKey::F15;
			case GLFW_KEY_F16:				return EKey::F16;
			case GLFW_KEY_F17:				return EKey::F17;
			case GLFW_KEY_F18:				return EKey::F18;
			case GLFW_KEY_F19:				return EKey::F19;
			case GLFW_KEY_F20:				return EKey::F20;
			case GLFW_KEY_F21:				return EKey::F21;
			case GLFW_KEY_F22:				return EKey::F22;
			case GLFW_KEY_F23:				return EKey::F23;
			case GLFW_KEY_F24:				return EKey::F24;
			case GLFW_KEY_F25:				return EKey::F25;

			case GLFW_KEY_KP_0:				return EKey::Numpad0;
			case GLFW_KEY_KP_1:				return EKey::Numpad1;
			case GLFW_KEY_KP_2:				return EKey::Numpad2;
			case GLFW_KEY_KP_3:				return EKey::Numpad3;
			case GLFW_KEY_KP_4:				return EKey::Numpad4;
			case GLFW_KEY_KP_5:				return EKey::Numpad5;
			case GLFW_KEY_KP_6:				return EKey::Numpad6;
			case GLFW_KEY_KP_7:				return EKey::Numpad7;
			case GLFW_KEY_KP_8:				return EKey::Numpad8;
			case GLFW_KEY_KP_9:				return EKey::Numpad9;
			case GLFW_KEY_KP_DECIMAL:		return EKey::NumpadDecimal;
			case GLFW_KEY_KP_DIVIDE:		return EKey::NumpadDivide;
			case GLFW_KEY_KP_MULTIPLY:		return EKey::NumpadMultiply;
			case GLFW_KEY_KP_SUBTRACT:		return EKey::NumpadSubtract;
			case GLFW_KEY_KP_ADD:			return EKey::NumpadAdd;
			case GLFW_KEY_KP_ENTER:			return EKey::NumpadEnter;
			case GLFW_KEY_KP_EQUAL:			return EKey::NumpadEqual;

			case GLFW_KEY_LEFT_SHIFT:		return EKey::LeftShift;
			case GLFW_KEY_LEFT_CONTROL:		return EKey::LeftControl;
			case GLFW_KEY_LEFT_ALT:			return EKey::LeftAlt;
			case GLFW_KEY_LEFT_SUPER:		return EKey::LeftSuper; // Command/Windows key
			case GLFW_KEY_RIGHT_SHIFT:		return EKey::RightShift;
			case GLFW_KEY_RIGHT_CONTROL:	return EKey::RightControl;
			case GLFW_KEY_RIGHT_ALT:		return EKey::RightAlt;
			case GLFW_KEY_RIGHT_SUPER:		return EKey::RightSuper; // Command/Windows key
			case GLFW_KEY_MENU:				return EKey::Menu; // List menu key

			default:						return EKey::Unknown;
		}
	}

	static const int MouseToCode[EMouse::Maximum]
	{
		GLFW_MOUSE_BUTTON_LEFT,
		GLFW_MOUSE_BUTTON_RIGHT,
		GLFW_MOUSE_BUTTON_MIDDLE,
		GLFW_MOUSE_BUTTON_4,
		GLFW_MOUSE_BUTTON_5,
		GLFW_MOUSE_BUTTON_6,
		GLFW_MOUSE_BUTTON_7,
		GLFW_MOUSE_BUTTON_8,
	};

	static EMouse CodeToMouse( int Action )
	{
		switch( Action )
		{
			case GLFW_MOUSE_BUTTON_LEFT:	return EMouse::LeftMouseButton;
			case GLFW_MOUSE_BUTTON_RIGHT:	return EMouse::RightMouseButton;
			case GLFW_MOUSE_BUTTON_MIDDLE:	return EMouse::MiddleMouseButton;

			case GLFW_MOUSE_BUTTON_4:		return EMouse::MouseButton4;
			case GLFW_MOUSE_BUTTON_5:		return EMouse::MouseButton5;
			case GLFW_MOUSE_BUTTON_6:		return EMouse::MouseButton6;
			case GLFW_MOUSE_BUTTON_7:		return EMouse::MouseButton7;
			case GLFW_MOUSE_BUTTON_8:		return EMouse::MouseButton8;
			default:						return EMouse::Unknown;
		}
	}
}
