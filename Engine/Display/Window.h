// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Rendering/Renderer.h"

#define IMGUI_ENABLED

struct ViewDimensions
{
	ViewDimensions()
	{
		
	}

	ViewDimensions( int Width, int Height )
	{
		this->Width = Width;
		this->Height = Height;
	}

	int Width = -1;
	int Height = -1;
};

struct SystemInformation
{
	int MemoryTotal = 0;
	int MemoryAvailable = 0;
	int MemoryUsed = 0;

	int VideoMemoryTotal = 0;
	int VideoMemoryAvailable = 0;
	int VideoMemoryUsed = 0;
};

class CWindow
{
public:
	void Create( const char* Title );
	void Resize( const ViewDimensions& Dimensions = ViewDimensions() );
	void Fullscreen( const bool Enable );
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

	void SetWindowless( const bool Enable );
	bool IsWindowless() const
	{
		return Windowless;
	}

	bool IsFullscreen() const;
	void SetVSYNC( const bool Enable );

private:
	struct GLFWmonitor* GetTargetMonitor();
	ViewDimensions GetMonitorDimensions( struct GLFWmonitor* Monitor );

	struct GLFWwindow* WindowHandle = nullptr;
	CRenderer Renderer;

	bool Initialized = false;
	bool ShowCursor = false;

	ViewDimensions CurrentDimensions;

	bool RenderingFrame = false;
	bool Windowless = false;

public:
	static CWindow& Get()
	{
		static CWindow StaticInstance;
		return StaticInstance;
	}

	static struct GLFWwindow* ThreadContext( const bool MakeCurrent = true );
private:
	CWindow() {};

	CWindow( CWindow const& ) = delete;
	void operator=( CWindow const& ) = delete;
};
