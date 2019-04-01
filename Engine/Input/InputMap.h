// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <stdint.h>

enum class EAction : uint32_t
{
	Unknown = 0,

	Release,
	Press,
	Repeat,

	Maximum
};

typedef std::underlying_type<EAction>::type EActionType;

enum class EKey : uint32_t
{
	Unknown = 0,

	Space,
	Apostrophe,
	Comma,
	Minus,
	Period,
	Slash,

	Number0,
	Number1,
	Number2,
	Number3,
	Number4,
	Number5,
	Number6,
	Number7,
	Number8,
	Number9,

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
	LeftBracket,
	Backslash,
	RightBracket,
	GraveAccent,
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

typedef std::underlying_type<EKey>::type EKeyType;

enum class EModifier : uint32_t
{
	Unknown = 0,

	Shift,
	Control,
	Alt,
	Super,

	Maximum
};

typedef std::underlying_type<EModifier>::type EModifierType;

enum class EMouse : uint32_t
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

typedef std::underlying_type<EMouse>::type EMouseType;

enum class ECursor : uint32_t
{
	Unknown = 0,

	Normal,
	Hidden,
	Disabled,

	Maximum
};

typedef std::underlying_type<ECursor>::type ECursorType;

enum class EGamepad : uint32_t
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

typedef std::underlying_type<EGamepad>::type EGamepadType;
