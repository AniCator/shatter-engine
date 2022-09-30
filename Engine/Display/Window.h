// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include "Rendering/Renderer.h"

#include <Engine/Utility/Singleton.h>

#define IMGUI_ENABLED

struct ViewDimensions
{
	ViewDimensions() = default;
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

class CWindow : public Singleton<CWindow>
{
public:
	void Create( const char* Title );
	void Resize( const ViewDimensions& Dimensions = ViewDimensions() );
	void Fullscreen( const bool Enable );
	void Borderless( const bool Enable );
	void Terminate();
	GLFWwindow* Handle() const;

	void ProcessInput();

	void BeginFrame();
	void RenderFrame();
	void SwapFrame();

	// Forces the current frame to render.
	void FlushFrame();

	bool Valid() const;
	bool ShouldClose() const;

	void EnableCursor( bool Enabled );
	bool IsCursorEnabled() const;
	void UpdateCursor() const;

	void SetIcon( class CTexture* Texture );
	void SetTitle( const std::string& Title );

	CRenderer& GetRenderer();

	int GetWidth() const { return CurrentDimensions.Width; };
	int GetHeight() const { return CurrentDimensions.Height; };

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
	bool IsBorderless() const;
	bool IsFullscreenBorderless() const;

	bool HasVSync() const;
	void SetVSync( const bool& Enable );
	void SetSwapInterval( const int Interval );

	bool IsFocused() const;
	bool IsMinimized() const;

	struct GLFWmonitor* GetTargetMonitor() const;
	ViewDimensions GetMonitorDimensions( struct GLFWmonitor* Monitor ) const;
	ViewDimensions GetMonitorDimensionsRaw( struct GLFWmonitor* Monitor ) const;
	std::vector<ViewDimensions> GetMonitorDimensionsAll( struct GLFWmonitor* Monitor ) const;

private:
	void Recreate();

	struct GLFWwindow* WindowHandle = nullptr;
	CRenderer Renderer;

	bool Initialized = false;
	bool ShowCursor = false;
	bool ShouldRecreate = false;

	ViewDimensions CurrentDimensions;

	bool RenderingFrame = false;
	bool Windowless = false;

	// Application icon
	GLFWimage Icon;

	std::string WindowTitle;

public:

	static struct GLFWwindow* ThreadContext( const bool MakeCurrent = true );
};
