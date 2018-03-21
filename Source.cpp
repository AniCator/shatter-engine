// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Display/Window.h>
#include <Engine/Configuration/Configuration.h>

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Input/Input.h>

#include <Engine/Application/Application.h>

#include <Game/Game.h>

#include <ThirdParty/imgui-1.52/imgui.h>

#ifdef ConsoleWindowDisabled
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

CWindow& MainWindow = CWindow::GetInstance();
static const char* pszWindowTitle = "AICritters Prototype 3";
static const int nBuildNumber = 0;

CCamera DefaultCamera = CCamera();
FCameraSetup& Setup = DefaultCamera.GetCameraSetup();
bool PauseGame = false;
bool ScaleTime = false;
bool CursorVisible = false;

void InputScaleTimeEnable()
{
	ScaleTime = true;
}

void InputScaleTimeDisable()
{
	ScaleTime = false;
}

void InputPauseGameEnable()
{
	PauseGame = true;
}

void InputPauseGameDisable()
{
	PauseGame = false;
}

void InputReloadConfiguration()
{
	CConfiguration::GetInstance().Reload();
}

void InputRestartGameLayers()
{
	GameLayersInstance->Shutdown();
	GameLayersInstance->Initialize();

	// Force frame refresh
	MainWindow.GetRenderer().RefreshFrame();
}

const float CameraSpeed = CConfiguration::GetInstance().GetFloat( "cameraspeed" );
void InputMoveCameraUp()
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[1] += Speed;
}

void InputMoveCameraDown()
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[1] -= Speed;
}

void InputMoveCameraLeft()
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[0] -= Speed;
}

void InputMoveCameraRight()
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[0] += Speed;
}

void InputMoveCameraLower()
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[2] -= Speed;
}

void InputMoveCameraHigher()
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[2] += Speed;
}

void InputToggleMouse()
{
	CursorVisible = !CursorVisible;

	MainWindow.EnableCursor( CursorVisible );
}

void Initialize()
{
	CConfiguration::GetInstance().Initialize();

	char szTitle[256];
	sprintf_s( szTitle, "%s (Build: %i)", pszWindowTitle, nBuildNumber );

	MainWindow.Create( szTitle );

	if( !MainWindow.Valid() )
	{
		Log::Event( Log::Fatal, "Application window could not be created.\n" );
	}

	GLFWwindow* WindowHandle = MainWindow.Handle();
	glfwSetKeyCallback( WindowHandle, InputKeyCallback );
	glfwSetCharCallback( WindowHandle, InputCharCallback );
	glfwSetMouseButtonCallback( WindowHandle, InputMouseButtonCallback );
	glfwSetScrollCallback( WindowHandle, InputScrollCallback );
	glfwSetJoystickCallback( InputJoystickStatusCallback );

	CInput& Input = CInput::GetInstance();

	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_SPACE, GLFW_PRESS, InputPauseGameEnable );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_SPACE, GLFW_RELEASE, InputPauseGameDisable );

	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_ENTER, GLFW_PRESS, InputScaleTimeEnable );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_ENTER, GLFW_RELEASE, InputScaleTimeDisable );

	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_H, GLFW_RELEASE, InputReloadConfiguration );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_G, GLFW_RELEASE, InputRestartGameLayers );
	Input.AddActionBinding( EActionBindingType::Mouse, GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, InputRestartGameLayers );

	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_W, GLFW_PRESS, InputMoveCameraUp );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_S, GLFW_PRESS, InputMoveCameraDown );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_A, GLFW_PRESS, InputMoveCameraLeft );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_D, GLFW_PRESS, InputMoveCameraRight );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_R, GLFW_PRESS, InputMoveCameraLower );
	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_F, GLFW_PRESS, InputMoveCameraHigher );

	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_ESCAPE, GLFW_RELEASE, InputToggleMouse );

	MainWindow.EnableCursor( CursorVisible );

	if( !GameLayersInstance )
	{
		Log::Event( Log::Fatal, "Game layers instance does not exist!\n" );
	}

	GameLayersInstance->RegisterGameLayers();
}

void DebugMenu()
{
	if( ImGui::BeginMainMenuBar() )
	{
		if( ImGui::BeginMenu( "Shatter 0.0.0" ) )
		{
			ImGui::MenuItem( "2017 \xc2\xa9 Christiaan Bakker", NULL, false, false );

			if( ImGui::MenuItem( "Quit", "Escape" ) )
			{
				glfwSetWindowShouldClose( MainWindow.Handle(), true );
			}

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Commands" ) )
		{
			if( ImGui::MenuItem( "Pause", NULL, PauseGame ) )
			{
				PauseGame = !PauseGame;
			}

			if( ImGui::MenuItem( "Slow Motion", NULL, ScaleTime ) )
			{
				ScaleTime = !ScaleTime;
			}

			ImGui::Separator();

			if( ImGui::MenuItem( "Reload Configuration", "H" ) )
			{
				InputReloadConfiguration();
			}

			if( ImGui::MenuItem( "Restart Game Layers", "G" ) )
			{
				InputRestartGameLayers();
			}

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Profiler" ) )
		{
			CProfileVisualisation& Profiler = CProfileVisualisation::GetInstance();
			const bool Enabled = Profiler.IsEnabled();
			if( ImGui::MenuItem( "Toggle", NULL, Enabled ) )
			{
				Profiler.SetEnabled( !Enabled );
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void main()
{
	Log::Event( "%s (Build: %i)\n\n", pszWindowTitle, nBuildNumber );
	Initialize();

	CRenderer& Renderer = MainWindow.GetRenderer();

	Renderer.Initialize();

	Setup.AspectRatio = static_cast<float>( MainWindow.GetWidth() ) / static_cast<float>( MainWindow.GetHeight() );
	Renderer.SetCamera( DefaultCamera );

	CTimer InputTimer( false );
	CTimer GameTimer( false );

	InputTimer.Start();
	GameTimer.Start();

	double ScaledGameTime = 0.0;

	GameLayersInstance->Initialize();

	const uint64_t MaximumFrameTime = 1000 / CConfiguration::GetInstance().GetInteger( "fps" );
	const uint64_t MaximumGameTime = 1000 / CConfiguration::GetInstance().GetInteger( "tickrate" );
	const uint64_t MaximumInputTime = 1000 / CConfiguration::GetInstance().GetInteger( "pollingrate" );

	while( !MainWindow.ShouldClose() )
	{
		MainWindow.BeginFrame();
		// Profile( "Frametime" );
		CTimerScope Scope_Frametime( "Frametime", false );

		const uint64_t InputDeltaTime = InputTimer.GetElapsedTimeMilliseconds();
		if( InputDeltaTime >= MaximumInputTime )
		{
			MainWindow.ProcessInput();

			InputTimer.Start();
		}

		const float TimeScale = ScaleTime ? 0.25f : 1.0f;

		const uint64_t GameDeltaTime = GameTimer.GetElapsedTimeMilliseconds();
		if( GameDeltaTime >= MaximumGameTime )
		{
			if( !PauseGame )
			{
				Renderer.RefreshFrame();

				const float TimeScaleParameter = CConfiguration::GetInstance().GetFloat( "timescale" );
				ScaledGameTime += TimeScale * TimeScaleParameter;

				Renderer.SetCamera( DefaultCamera );

				// Update time
				GameLayersInstance->Time( ScaledGameTime );
				GameLayersInstance->SetTimeScale( TimeScale * TimeScaleParameter );

				// Tick all game layers
				GameLayersInstance->Tick();

				GameTimer.Start();
			}
		}

		if( PauseGame )
		{
			GameTimer.Start();
		}

		{
			GameLayersInstance->Frame();

#ifdef IMGUI_ENABLED
			CProfileVisualisation::GetInstance().Display();

			DebugMenu();
#endif

			MainWindow.RenderFrame();
		}
	}

	GameLayersInstance->Shutdown();
	delete GameLayersInstance;

	Log::Event( "Meshes created: %i\n", MainWindow.GetRenderer().MeshCount() );

	MainWindow.Terminate();
}
