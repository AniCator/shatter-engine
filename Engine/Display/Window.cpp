// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Window.h"

#include <stdint.h>

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.3.2.bin.WIN64/include/GLFW/glfw3.h>

#include <Engine/Application/Application.h>
#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>

#include <Engine/Utility/Locator/InputLocator.h>

#include <Engine/Utility/TranslationTable.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.70/imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#endif

#if defined(_DEBUG)
#define KHRDebug
#else
// #define KHRDebug // This crashes people when the debug context happens to be enabled on some systems.
#endif

static const auto GLSourceTable = Translate<GLenum, std::string>( {
	{ GL_DEBUG_SOURCE_API, "API"},
	{ GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Window System" },
	{ GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader Compiler" },
	{ GL_DEBUG_SOURCE_THIRD_PARTY, "Third Party" },
	{ GL_DEBUG_SOURCE_APPLICATION, "Application" },
	{ GL_DEBUG_SOURCE_OTHER, "Other" }
	} 
);

static const auto GLSeverityTable = Translate<GLenum, std::string>( {
	{ GL_DEBUG_SEVERITY_NOTIFICATION, "Info"},
	{ GL_DEBUG_SEVERITY_LOW, "Low" },
	{ GL_DEBUG_SEVERITY_MEDIUM, "Medium" },
	{ GL_DEBUG_SEVERITY_HIGH, "High" }
	}
);

static void DebugCallbackOpenGL( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam )
{
	if( severity != GL_DEBUG_SEVERITY_HIGH )
		return;

	const auto Source = GLSourceTable.To( source );
	const auto Severity = GLSeverityTable.To( severity );
	if( type == GL_DEBUG_TYPE_ERROR )
	{
		Log::Event( Log::Error, "OpenGL %s Error (%u) (%s):\n\t%s\n", Source.c_str(), id, Severity.c_str(), message );
	}
	else if ( type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR )
	{
		Log::Event( Log::Warning, "OpenGL %s Deprecated Behavior (%u) (%s):\n\t%s\n", Source.c_str(), id, Severity.c_str(), message );
	}
	else if( type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR )
	{
		Log::Event( Log::Warning, "OpenGL %s Undefined Behavior (%u) (%s):\n\t%s\n", Source.c_str(), id, Severity.c_str(), message );
	}
	else if( type == GL_DEBUG_TYPE_PERFORMANCE )
	{
		Log::Event( Log::Warning, "OpenGL %s Performance (%u) (%s):\n\t%s\n", Source.c_str(), id, Severity.c_str(), message );
	}
}

void DebugCallbackGLFW( int error, const char* description )
{
	Log::Event( "GLFW: %s\n", description );
}

void CWindow::Create( const char* Title )
{
	ProfileBare( __FUNCTION__ );

	// Load configuration data
	CConfiguration& config = CConfiguration::Get();

	const bool EnableBorder = !config.IsEnabled( "noborder", true );

	CurrentDimensions.Width = config.GetInteger( "width", -1 );
	CurrentDimensions.Height = config.GetInteger( "height", -1 );

	// Ignore tiny window dimensions.
	if( CurrentDimensions.Width < 32 || CurrentDimensions.Height < 32 )
	{
		CurrentDimensions.Width = -1;
		CurrentDimensions.Height = -1;
	}

	Icon.pixels = nullptr;

	// Make sure GLFW is terminated before initializing it in case the application is being re-initialized.
	glfwTerminate();

	if( IsWindowless() )
		return;

	WindowTitle = Title;

	if( !glfwInit() )
	{
		Log::Event( Log::Fatal, "Failed to initialize GLFW\n" );
	}

	glfwSetErrorCallback( DebugCallbackGLFW );

	glfwWindowHint( GLFW_RESIZABLE, false );
	glfwWindowHint( GLFW_DECORATED, EnableBorder );

	glfwWindowHint( GLFW_SAMPLES, 0 );
	const bool DebugContext = config.IsEnabled( "opengldebugcontext", false );
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, DebugContext ? 1 : 0 );

	const int MajorVersion = 4; // config.GetInteger( "openglversionmajor", 4 );
	const int MinorVersion = 3; // config.GetInteger( "openglversionminor", 3 );

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

	// Increase the height by 1 pixel to avoid the exclusive no border behavior.
	if( !EnableBorder )
	{
		CurrentDimensions.Height += 1;
	}

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

	glfwSetInputMode( WindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

	glfwMakeContextCurrent( WindowHandle );
	
	gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress );

	Log::Event( "OpenGL %s\n", glGetString( GL_VERSION ) );

#if defined(KHRDebug)
	if( DebugContext && GLAD_GL_KHR_debug )
	{
		Log::Event( "KHR debug extention enabled.\n" );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR );
		glDebugMessageControlKHR( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_KHR, 0, nullptr, true );
		glDebugMessageCallbackKHR( DebugCallbackOpenGL, nullptr );
	}
#else
	if( DebugContext )
	{
		Log::Event( "Standard debug extention enabled.\n" );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
		/*glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, true );
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, false );
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, false );
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, false );*/
		glDebugMessageCallback( DebugCallbackOpenGL, nullptr );
	}
#endif

	const auto EnableSync = config.GetInteger( "vsync", 0 ) > 0;
	SetVSync( EnableSync );

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

void CWindow::Recreate()
{
	return; // TODO: Causes issues with ImGui.

	ShouldRecreate = false;

	if( IsWindowless() )
		return;

	glfwWindowHint( GLFW_VISIBLE, GL_FALSE );

	// auto* Context = glfwCreateWindow( 1, 1, "RecreateContext", NULL, NULL );
	auto* Context = ThreadContext( true );
	// glfwMakeContextCurrent( Context );

	glfwDestroyWindow( WindowHandle );

	const bool EnableBorder = !CConfiguration::Get().IsEnabled( "noborder", true );
	const bool FullScreen = EnableBorder && CConfiguration::Get().IsEnabled( "fullscreen", false );

	glfwWindowHint( GLFW_RESIZABLE, false );
	glfwWindowHint( GLFW_DECORATED, EnableBorder );
	// glfwWindowHint( GLFW_FLOATING, FullScreen );

	auto* Monitor = GetTargetMonitor();
	WindowHandle = glfwCreateWindow( 
		CurrentDimensions.Width, CurrentDimensions.Height, 
		WindowTitle.c_str(), 
		FullScreen ? Monitor : nullptr, 
		Context 
	);

	glfwMakeContextCurrent( WindowHandle );

	if( !WindowHandle )
	{
		Log::Event( Log::Fatal, "Failed to recreate the window.\n" );
	}

#if defined( IMGUI_ENABLED )
	CApplication::ResetImGui();
#endif
}

void CWindow::Resize( const ViewDimensions& Dimensions )
{
	if( IsWindowless() ) 
		return;

	ViewDimensions NewDimensions;
	if( Dimensions.Width > 0 && Dimensions.Height > 0 )
	{
		NewDimensions = Dimensions;
	}
	else
	{
		auto* Monitor = GetTargetMonitor();
		NewDimensions = GetMonitorDimensions( Monitor );
	}

	// Nudge the height so that the borderless window isn't made exclusive by the OS/graphics driver.
	if( IsBorderless() )
	{
		NewDimensions.Height += 1;
	}

	if( CurrentDimensions.Width != NewDimensions.Width && CurrentDimensions.Height != NewDimensions.Height )
	{
		CurrentDimensions = NewDimensions;
		glfwSetWindowSize( WindowHandle, CurrentDimensions.Width, CurrentDimensions.Height );

		auto& Configuration = CConfiguration::Get();
		Configuration.Store( "width", CurrentDimensions.Width );
		Configuration.Store( "height", CurrentDimensions.Height );
		Configuration.Save();

		Renderer.DestroyBuffers();

		glfwFocusWindow( WindowHandle );
	}
}

void CWindow::Fullscreen( const bool Enable )
{
	if( IsWindowless() )
		return;

	const int AsInteger = Enable ? 1 : 0;

	CConfiguration::Get().Store( "fullscreen", AsInteger );
	CConfiguration::Get().Store( "noborder", 0 );
	CConfiguration::Get().Save();

	GLFWmonitor* Monitor = GetTargetMonitor();

	if( Enable )
	{
		const GLFWvidmode* VideoMode = glfwGetVideoMode( Monitor );
		const auto Dimensions = ViewDimensions( VideoMode->width, VideoMode->height );

		glfwSetWindowMonitor( WindowHandle, Monitor, 0, 0, Dimensions.Width, Dimensions.Height, VideoMode->refreshRate );
		Resize( Dimensions );
	}
	else
	{
		const auto Dimensions = GetMonitorDimensions( Monitor );

		glfwSetWindowMonitor( WindowHandle, nullptr, 100, 100, Dimensions.Width, Dimensions.Height, GLFW_DONT_CARE );
	}

	glfwSetWindowAttrib( WindowHandle, GLFW_FLOATING, AsInteger );
	glfwSetWindowAttrib( WindowHandle, GLFW_DECORATED, 1 );
}

void CWindow::Borderless( const bool Enable )
{
	if( IsWindowless() )
		return;

	const int AsInteger = Enable ? 1 : 0;

	auto& Configuration = CConfiguration::Get();
	Configuration.Store( "fullscreen", 0 );
	Configuration.Store( "noborder", AsInteger );
	Configuration.Save();

	if( Enable )
	{
		Configuration.Store( "width", -1 );
		Configuration.Store( "height", -1 );
	}

	GLFWmonitor* Monitor = GetTargetMonitor();
	const auto Dimensions = GetMonitorDimensions( Monitor );

	const int Offset = Enable ? 0 : 100;

	glfwSetWindowMonitor( WindowHandle, nullptr, Offset, Offset, Dimensions.Width, Dimensions.Height + 1, GLFW_DONT_CARE );

	glfwSetWindowAttrib( WindowHandle, GLFW_FLOATING, 0 );
	glfwSetWindowAttrib( WindowHandle, GLFW_DECORATED, 1 - AsInteger );

	// Nudge the window by resizing it twice so that it is on top.
	Resize( ViewDimensions( 800, 600 ) );
	Resize( Dimensions );

	glfwFocusWindow( WindowHandle );
}

void CWindow::Terminate()
{
	if( IsWindowless() )
		return;

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
	if( IsWindowless() )
		return;

	// ProfileAlways( "Input" );
	// OptickCategory( "Input", Optick::Category::Input );

	glfwPollEvents();

	// CInputLocator::Get().Tick();
}

void CWindow::BeginFrame()
{
	if( IsRendering() || IsWindowless() )
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
	if( !IsRendering() || IsWindowless() )
		return;

	ProfileAlways( "Render" );
	OptickCategory( "Render", Optick::Category::Rendering );

#if defined(IMGUI_ENABLED)
	{
		OptickEvent( "UI Frame" );
		UI::Frame();
	}
#endif

	Renderer.SetViewport( CurrentDimensions.Width, CurrentDimensions.Height );
	Renderer.DrawQueuedRenderables();

#if defined( IMGUI_ENABLED )
	{
		OptickEvent( "ImGUI" );
		ImGui::Render();
	}

	{
		OptickEvent( "UI Render" );
		UI::Render();
	}

	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#endif

	RenderingFrame = false;

	if( ShouldRecreate )
	{
		Recreate();
	}
}

void CWindow::SwapFrame()
{
	OptickEvent( "Swap Buffers" );
	glfwSwapBuffers( WindowHandle );
}

void CWindow::FlushFrame()
{
	RenderFrame();
	Renderer.RefreshFrame();
	BeginFrame();
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

void CWindow::UpdateCursor() const
{
	static bool PreviousCursorState = false;
	if( PreviousCursorState != ShowCursor )
	{
		glfwSetInputMode( WindowHandle, GLFW_CURSOR, ShowCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
		PreviousCursorState = ShowCursor;
	}
}

void CWindow::SetIcon( CTexture* Texture )
{
	if( !Texture )
		return;

	const auto Format = Texture->GetImageFormat();
	if( Format != EImageFormat::RGBA8 )
		return;
	
	auto* Data = Texture->GetImageData();
	if( !Data )
		return;

	Icon.width = Texture->GetWidth();
	Icon.height = Texture->GetHeight();

	// Remove existing icon.
	delete [] Icon.pixels;

	auto* UnsignedData = StaticCast<unsigned char>( Data );
	const size_t Size = Icon.width * Icon.height * 4;
	
	Icon.pixels = new unsigned char[Size];
	for( size_t Index = 0; Index < Size; )
	{
		Icon.pixels[Index + 0] = UnsignedData[Index + 0];
		Icon.pixels[Index + 1] = UnsignedData[Index + 1];
		Icon.pixels[Index + 2] = UnsignedData[Index + 2];
		Icon.pixels[Index + 3] = UnsignedData[Index + 3];

		Index += 4;
	}

	glfwSetWindowIcon( WindowHandle, 1, &Icon );
}

void CWindow::SetTitle( const std::string& Title )
{
	glfwSetWindowTitle( WindowHandle, Title.c_str() );
}

CRenderer& CWindow::GetRenderer()
{
	return Renderer;
}

void CWindow::SetWindowless( const bool Enable )
{
	Windowless = true;
}

bool CWindow::IsFullscreen() const
{
	const auto Monitor = glfwGetWindowMonitor( WindowHandle );
	return Monitor != nullptr;
}

bool CWindow::IsBorderless() const
{
	return glfwGetWindowAttrib( WindowHandle, GLFW_DECORATED ) == 0;
}

bool CWindow::IsFullscreenBorderless() const
{
	auto* Monitor = GetTargetMonitor();
	const auto Dimensions = GetMonitorDimensions( Monitor );
	const bool Match = Dimensions.Width == CurrentDimensions.Width && Dimensions.Height == CurrentDimensions.Height;
	return IsBorderless() && Match;
}

bool CWindow::HasVSync() const
{
	return CConfiguration::Get().IsEnabled( "vsync" );
}

void CWindow::SetVSync( const bool& Enable )
{
	const int Interval = Enable ? 1 : 0;
	CConfiguration::Get().Store( "vsync", Interval );
	SetSwapInterval( Interval );
}

void CWindow::SetSwapInterval( const int Interval )
{
	if( Interval != 0 )
	{
		if( glfwExtensionSupported( "WGL_EXT_swap_control_tear" ) || 
			glfwExtensionSupported( "GLX_EXT_swap_control_tear" ) )
		{
			// Ensure the interval is negative to enable adaptive sync.
			glfwSwapInterval( abs( Interval ) * -1 );
			return;
		}
	}

	glfwSwapInterval( Interval );
}

bool CWindow::IsFocused() const
{
	return glfwGetWindowAttrib( WindowHandle, GLFW_FOCUSED ) == 1;
}

bool CWindow::IsMinimized() const
{
	return glfwGetWindowAttrib( WindowHandle, GLFW_ICONIFIED ) == 1;
}

GLFWmonitor* CWindow::GetTargetMonitor() const
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

ViewDimensions CWindow::GetMonitorDimensions( GLFWmonitor* Monitor ) const
{
	auto& Configuration = CConfiguration::Get();

	ViewDimensions Dimensions;
	Dimensions.Width = Configuration.GetInteger( "width", -1 );
	Dimensions.Height = Configuration.GetInteger( "height", -1 );

	if( Dimensions.Width < 32 || Dimensions.Height < 32 )
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
