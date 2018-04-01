// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Window.h"

#include <stdint.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Configuration/Configuration.h>

#include <Engine/Input/Input.h>

#ifdef IMGUI_ENABLED
#include <ThirdParty/imgui-1.52/imgui.h>
#include "imgui_impl_glfw_gl3.h"
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
	CConfiguration& config = CConfiguration::GetInstance();

	const bool EnableBorder = !config.IsEnabled( "noborder" );

	Width = 1280;
	Height = 720;

	if( config.IsValidKey( "width" ) )
	{
		Width = config.GetInteger( "width" );
	}

	if( config.IsValidKey( "height" ) )
	{
		Height = config.GetInteger( "height" );
	}

	if( !glfwInit() )
	{
		Log::Event( Log::Fatal, "Failed to initialize GLFW\n" );
	}

	glfwSetErrorCallback( DebugCallbackGLFW );

	glfwWindowHint( GLFW_RESIZABLE, false );
	glfwWindowHint( GLFW_DECORATED, EnableBorder );

	glfwWindowHint( GLFW_SAMPLES, config.GetInteger( "aasamples" ) );
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, config.IsEnabled( "opengldebugcontext" ) );

	if( config.IsValidKey( "openglversionmajor" ) && config.IsValidKey( "openglversionminor" ) )
	{
		const int MajorVersion = config.GetInteger( "openglversionmajor" );
		const int MinorVersion = config.GetInteger( "openglversionminor" );

		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, MajorVersion );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, MinorVersion );
	}

	if( config.IsValidKey( "openglcore" ) && config.IsEnabled( "openglcore" ) )
	{
		glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
		glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	}

	const bool FullScreen = EnableBorder && config.IsEnabled( "fullscreen" );
	const int TargetMonitor = config.GetInteger( "monitor" );

	if( FullScreen )
	{
		const GLFWvidmode* VideoMode = glfwGetVideoMode( glfwGetPrimaryMonitor() );
		glfwWindowHint( GLFW_RED_BITS, VideoMode->redBits );
		glfwWindowHint( GLFW_GREEN_BITS, VideoMode->greenBits );
		glfwWindowHint( GLFW_BLUE_BITS, VideoMode->blueBits );
		glfwWindowHint( GLFW_REFRESH_RATE, VideoMode->refreshRate );
	}

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

	WindowHandle = glfwCreateWindow( Width, Height, Title, FullScreen ? glfwGetPrimaryMonitor() : nullptr, nullptr );

	int WindowX = -1;
	int WindowY = -1;

	if( config.IsValidKey( "windowx" ) )
	{
		WindowX = config.GetInteger( "windowx" );
	}

	if( config.IsValidKey( "windowy" ) )
	{
		WindowY = config.GetInteger( "windowy" );
	}

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

	glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR );
	glDebugMessageControlKHR( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_KHR, 0, nullptr, true );
	glDebugMessageCallbackKHR( DebugCallbackOpenGL, nullptr );

	glfwSwapInterval( config.GetInteger( "vsync" ) );

#ifdef IMGUI_ENABLED
	ImGui_ImplGlfwGL3_Init( WindowHandle, false );
#endif

	Initialized = true;
}

void CWindow::Terminate()
{
#ifdef IMGUI_ENABLED
	ImGui_ImplGlfwGL3_Shutdown();
#endif

	glfwTerminate();
}

GLFWwindow* CWindow::Handle() const
{
	return WindowHandle;
}

void CWindow::ProcessInput()
{
	glfwPollEvents();

	glfwSetInputMode( WindowHandle, GLFW_CURSOR, ShowCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );

	if( !ShowCursor )
	{
		glfwSetCursorPos( WindowHandle, Width * 0.5, Height * 0.5 );
	}

	CInput::GetInstance().Tick();
}

void CWindow::BeginFrame()
{
#ifdef IMGUI_ENABLED
	ImGui_ImplGlfwGL3_NewFrame();
#endif
}

void CWindow::RenderFrame()
{
	Profile( "Render" );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	Renderer.DrawQueuedRenderables();

#ifdef IMGUI_ENABLED
	ImGui::Render();
#endif

	glfwSwapBuffers( WindowHandle );
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
