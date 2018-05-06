// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>

enum class EInputMappingActionType : uint32_t
{
	Unknown = 0,

	Release,
	Press,
	Repeat,

	Maximum
};

enum class EInputMappingKeyType : uint32_t
{
	Unknown = 0,

	Space,
	Apostrophe,
	Comma,
	Minus,
	Period,
	Slash,

	Num0,
	Num1,
	Num2,
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,

	Semicolon,
	Equal,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Left_bracket,
	Backslash,
	Right_bracket,
	Grave_accent,
	World1, // Non-US Key 1
	World2, // Non-US Key 2

	Escape,
	Enter,
	Tab,
	Backspace,
	Insert,
	Delete,
	Right,
	Left,
	Down,
	Up,
	PageUp,
	PageDown,
	Home,
	End,
	CapsLock,
	ScrollLock,
	NumLock,
	PrintScreen,
	Pause,

	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,
	F25,

	Numpad0,
	Numpad1,
	Numpad2,
	Numpad3,
	Numpad4,
	Numpad5,
	Numpad6,
	Numpad7,
	Numpad8,
	Numpad9,
	NumpadDecimal,
	NumpadDivide,
	NumpadMultiply,
	NumpadSubtract,
	NumpadAdd,
	NumpadEnter,
	NumpadEqual,

	LeftShift,
	LeftControl,
	LeftAlt,
	LeftSuper, // Command/Windows key
	RightShift,
	RightControl,
	RightAlt,
	RightSuper, // Command/Windows key
	Menu, // List menu key

	Maximum
};

enum class EInputMappingModifierType : uint32_t
{
	Unknown = 0,

	Shift,
	Control,
	Alt,
	Super,

	Maximum
};

enum class EInputMappingMouseType : uint32_t
{
	Unknown = 0,

	MouseX,
	MouseY,

	MouseScrollUp,
	MouseScrollDown,

	LeftMouseButton,
	RightMouseButton,
	MiddleMouseButton,

	MouseButton4,
	MouseButton5,
	MouseButton6,
	MouseButton7,
	MouseButton8,

	Maximum
};

enum class EInputMappingJoystickType : uint32_t
{
	Unknown = 0,

	JoystickButton1,
	JoystickButton2,
	JoystickButton3,
	JoystickButton4,
	JoystickButton5,
	JoystickButton6,
	JoystickButton7,
	JoystickButton8,
	JoystickButton9,
	JoystickButton10,
	JoystickButton11,
	JoystickButton12,
	JoystickButton13,
	JoystickButton14,
	JoystickButton15,
	JoystickButton16,

	Maximum
};

enum class EInputMappingGamepadType : uint32_t
{
	Unknown = 0,

	GamepadFaceButtonUp,
	GamepadFaceButtonDown,
	GamepadFaceButtonLeft,
	GamepadFaceButtonRight,

	GamepadDirectionalButtonUp,
	GamepadDirectionalButtonDown,
	GamepadDirectionalButtonLeft,
	GamepadDirectionalButtonRight,

	GamepadLeftStickX,
	GamepadLeftStickY,
	GamepadLeftStickTrigger,

	GamepadLeftTrigger,
	GamepadLeftShoulder,
	GamepadLeftSpecial,

	GamepadRightStickX,
	GamepadRightStickY,
	GamepadRightStickTrigger,

	GamepadRightTrigger,
	GamepadRightShoulder,
	GamepadRightSpecial,

	Maximum
};
