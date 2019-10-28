// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Window.h"

#include <stdint.h>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW/glfw3.h>

#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Utility/Locator/InputLocator.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.70/imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#endif

static void DebugCallbackOpenGL( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam )
{
	if( type == GL_DEBUG_TYPE_ERROR )
	{
		Log::Event( "OpenGL: %s\n", message );
	}
}

void DebugCallbackGLFW( int error, const char* description )
{
	Log::Event( "GLFW: %s\n", description );
}

CWindow::CWindow()
{
	Initialized = false;
	ShowCursor = false;
}

void CWindow::Create( const char* Title )
{
	ProfileBare( __FUNCTION__ );

	// Load configuration data
	CConfiguration& config = CConfiguration::Get();

	const bool EnableBorder = !config.IsEnabled( "noborder", false );

	CurrentDimensions.Width = config.GetInteger( "width", -1 );
	CurrentDimensions.Height = config.GetInteger( "height", -1 );

	// Make sure GLFW is terminated before initializing it in case the application is being re-initialized.
	glfwTerminate();
	if( !glfwInit() )
	{
		Log::Event( Log::Fatal, "Failed to initialize GLFW\n" );
	}

	glfwSetErrorCallback( DebugCallbackGLFW );

	glfwWindowHint( GLFW_RESIZABLE, false );
	glfwWindowHint( GLFW_DECORATED, EnableBorder );

	glfwWindowHint( GLFW_SAMPLES, 0 );
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, config.IsEnabled( "opengldebugcontext", false ) );

	const int MajorVersion = config.GetInteger( "openglversionmajor", 3 );
	const int MinorVersion = config.GetInteger( "openglversionminor", 3 );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, MajorVersion );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, MinorVersion );

	if( config.IsEnabled( "openglcore", true ) )
	{
		glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
		glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	}

	const bool FullScreen = EnableBorder && config.IsEnabled( "fullscreen", false );

	if( FullScreen )
	{
		const GLFWvidmode* VideoMode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
		glfwWindowHint( GLFW_RED_BITS, VideoMode->redBits );
		glfwWindowHint( GLFW_GREEN_BITS, VideoMode->greenBits );
		glfwWindowHint( GLFW_BLUE_BITS, VideoMode->blueBits );
		glfwWindowHint( GLFW_REFRESH_RATE, VideoMode->refreshRate );
	}

	auto Context = ThreadContext( false );

	GLFWmonitor* Monitor = GetTargetMonitor();
	CurrentDimensions = GetMonitorDimensions( Monitor );

	glfwWindowHint( GLFW_VISIBLE, GL_TRUE );
	WindowHandle = glfwCreateWindow( CurrentDimensions.Width, CurrentDimensions.Height, Title, FullScreen ? Monitor : nullptr, Context );

	int WindowX = -1;
	int WindowY = -1;

	WindowX = config.GetInteger( "windowx", -1 );
	WindowY = config.GetInteger( "windowy", -1 );

	bool ShouldOverrideWindowPosition = false;

	if( WindowX > -1 || WindowY > -1 )
	{
		ShouldOverrideWindowPosition = true;
	}

	int MonitorX = 0;
	int MonitorY = 0;
	glfwGetMonitorPos( Monitor, &MonitorX, &MonitorY );

	if( !EnableBorder )
	{
		WindowX = MonitorX;
		WindowY = MonitorY;

		ShouldOverrideWindowPosition = true;
	}
	else
	{
		WindowX += MonitorX;
		WindowY += MonitorY;
	}

	if( ShouldOverrideWindowPosition )
	{
		glfwSetWindowPos( WindowHandle, WindowX, WindowY );
	}

	glfwMakeContextCurrent( WindowHandle );
	
	gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress );

	Log::Event( "OpenGL %s\n", glGetString( GL_VERSION ) );

#if defined(_DEBUG)
	if( GLAD_GL_KHR_debug )
	{
		Log::Event( "KHR debug extention enabled.\n" );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR );
		glDebugMessageControlKHR( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_KHR, 0, nullptr, true );
		glDebugMessageCallbackKHR( DebugCallbackOpenGL, nullptr );
	}
#endif

	const int SwapInterval = config.GetInteger( "vsync", 0 );
	Log::Event( "Swap interval: %i\n", SwapInterval );
	glfwSwapInterval( SwapInterval );

	Log::Event( "Initializing ImGui.\n" );
#if defined( IMGUI_ENABLED )
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL( WindowHandle, false );
	ImGui_ImplOpenGL3_Init( "#version 130" );
#endif

	Initialized = true;
	Log::Event( "Initialized window.\n" );

	Renderer.Initialize();
}

void CWindow::Resize( const ViewDimensions& Dimensions )
{
	ViewDimensions NewDimensions;
	if( Dimensions.Width > -1 && Dimensions.Height > -1 )
	{
		NewDimensions = Dimensions;
	}
	else
	{
		auto Monitor = GetTargetMonitor();
		NewDimensions = GetMonitorDimensions( Monitor );
	}

	if( CurrentDimensions.Width != NewDimensions.Width && CurrentDimensions.Height != NewDimensions.Height )
	{
		CurrentDimensions = NewDimensions;
		glfwSetWindowSize( WindowHandle, CurrentDimensions.Width, CurrentDimensions.Height );

		auto& Configuration = CConfiguration::Get();
		Configuration.Store( "width", CurrentDimensions.Width );
		Configuration.Store( "height", CurrentDimensions.Height );

		Renderer.DestroyBuffers();
	}
}

void CWindow::Terminate()
{
#if defined( IMGUI_ENABLED )
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#endif

	glfwTerminate();
}

GLFWwindow* CWindow::Handle() const
{
	return WindowHandle;
}

void CWindow::ProcessInput()
{
	Profile( "Input" );

	glfwPollEvents();

	static bool PreviousCursorState = false;
	if( PreviousCursorState != ShowCursor )
	{
		glfwSetInputMode( WindowHandle, GLFW_CURSOR, ShowCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
		PreviousCursorState = ShowCursor;
	}

	CInputLocator::Get().Tick();
}

void CWindow::BeginFrame()
{
	if( RenderingFrame )
		return;

	RenderingFrame = true;

#if defined( IMGUI_ENABLED )
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
#endif
}

void CWindow::RenderFrame()
{
	if( !RenderingFrame )
		return;

	Profile( "Render" );

#if defined(IMGUI_ENABLED)
	UI::Frame();
#endif

	Renderer.SetViewport( CurrentDimensions.Width, CurrentDimensions.Height );
	Renderer.DrawQueuedRenderables();

#if defined( IMGUI_ENABLED )
	ImGui::Render();

	UI::Render();

	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#endif

	glfwSwapBuffers( WindowHandle );

	RenderingFrame = false;
}

bool CWindow::Valid() const
{
	return Initialized;
}

bool CWindow::ShouldClose() const
{
	return glfwWindowShouldClose( WindowHandle ) > 0;
}

void CWindow::EnableCursor( bool Enabled )
{
	ShowCursor = Enabled;
}

bool CWindow::IsCursorEnabled() const
{
	return ShowCursor;
}

CRenderer& CWindow::GetRenderer()
{
	return Renderer;
}

GLFWmonitor* CWindow::GetTargetMonitor()
{
	const int TargetMonitor = CConfiguration::Get().GetInteger( "monitor", -1 );

	GLFWmonitor* Monitor = glfwGetPrimaryMonitor();
	if( TargetMonitor > -1 )
	{
		int MonitorCount;
		GLFWmonitor** Monitors = glfwGetMonitors( &MonitorCount );

		if( TargetMonitor < MonitorCount )
		{
			Monitor = Monitors[TargetMonitor];
		}
	}

	return Monitor;
}

ViewDimensions CWindow::GetMonitorDimensions( GLFWmonitor* Monitor )
{
	auto& Configuration = CConfiguration::Get();

	ViewDimensions Dimensions;
	Dimensions.Width = Configuration.GetInteger( "width", -1 );
	Dimensions.Height = Configuration.GetInteger( "height", -1 );

	if( Dimensions.Width == -1 && Dimensions.Height == -1 )
	{
		auto VideoMode = glfwGetVideoMode( Monitor );
		if( VideoMode )
		{
			Dimensions.Width = VideoMode->width;
			Dimensions.Height = VideoMode->height;

			Configuration.Store( "width", Dimensions.Width );
			Configuration.Store( "height", Dimensions.Height );
		}
	}

	return Dimensions;
}

GLFWwindow* CWindow::ThreadContext( const bool MakeCurrent )
{
	static GLFWwindow* Context = nullptr;
	if( !Context )
	{
		glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
		Context = glfwCreateWindow( 1, 1, "ThreadContext", NULL, NULL );
	}

	if( Context && MakeCurrent )
	{
		glfwMakeContextCurrent( Context );
	}

	return Context;
}
