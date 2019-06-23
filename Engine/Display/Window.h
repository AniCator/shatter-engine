// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Rendering/Renderer.h"

#define IMGUI_ENABLED

struct GLFWwindow;

class CWindow
{
public:
	void Create( const char* Title );
	void Terminate();
	GLFWwindow* Handle() const;

	void ProcessInput();

	void BeginFrame();
	void RenderFrame();

	bool Valid() const;
	bool ShouldClose() const;

	void EnableCursor( bool Enabled );
	bool IsCursorEnabled() const;

	CRenderer& GetRenderer();

	inline int GetWidth() { return Width; };
	inline int GetHeight() { return Height; };

	bool IsRendering() const
	{
		return RenderingFrame;
	}

private:
	GLFWwindow* WindowHandle;
	CRenderer Renderer;

	bool Initialized;
	bool ShowCursor;

	int Width;
	int Height;

	bool RenderingFrame;

public:
	static CWindow& Get()
	{
		static CWindow StaticInstance;
		return StaticInstance;
	}

	static GLFWwindow* ThreadContext( const bool MakeCurrent = true );
private:
	CWindow();

	CWindow( CWindow const& ) = delete;
	void operator=( CWindow const& ) = delete;
};
