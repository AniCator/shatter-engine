// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Rendering/Renderer.h"

#define IMGUI_ENABLED

struct ViewDimensions
{
	ViewDimensions()
	{
		Width = -1;
		Height = -1;
	}

	ViewDimensions( int Width, int Height )
	{
		this->Width = Width;
		this->Height = Height;
	}

	int Width;
	int Height;
};

class CWindow
{
public:
	void Create( const char* Title );
	void Resize( const ViewDimensions& Dimensions = ViewDimensions() );
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

	inline int GetWidth() const { return CurrentDimensions.Width; };
	inline int GetHeight() const { return CurrentDimensions.Height; };

	bool IsRendering() const
	{
		return RenderingFrame;
	}

private:
	struct GLFWmonitor* GetTargetMonitor();
	ViewDimensions GetMonitorDimensions( struct GLFWmonitor* Monitor );

	struct GLFWwindow* WindowHandle;
	CRenderer Renderer;

	bool Initialized;
	bool ShowCursor;

	ViewDimensions CurrentDimensions;

	bool RenderingFrame;

public:
	static CWindow& Get()
	{
		static CWindow StaticInstance;
		return StaticInstance;
	}

	static struct GLFWwindow* ThreadContext( const bool MakeCurrent = true );
private:
	CWindow();

	CWindow( CWindow const& ) = delete;
	void operator=( CWindow const& ) = delete;
};
