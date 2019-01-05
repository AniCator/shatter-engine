// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Application.h"

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Display/Window.h>
#include <Engine/Configuration/Configuration.h>

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Utility/Locator/InputLocator.h>

#include <Game/Game.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.52/imgui.h>
#include <Engine/Display/imgui_impl_glfw_gl3.h>
#endif

CWindow& MainWindow = CWindow::GetInstance();
static const int nBuildNumber = 0;

CCamera DefaultCamera = CCamera();
FCameraSetup& Setup = DefaultCamera.GetCameraSetup();
bool PauseGame = false;
bool ScaleTime = false;
bool CursorVisible = true;

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

void InputReloadShaders()
{
	MainWindow.GetRenderer().ReloadShaders();
}

float CameraSpeed = -1.0f;
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

std::map<std::string, std::function<void()>> Themes;

auto CherryHigh = [] ( float v ) { return ImVec4( 0.502f, 0.075f, 0.256f, v ); };
auto CherryMedium = [] ( float v ) { return ImVec4( 0.455f, 0.198f, 0.301f, v ); };
auto CherryLow = [] ( float v ) { return ImVec4( 0.232f, 0.201f, 0.271f, v ); };

auto CherryBackground = [] ( float v ) { return ImVec4( 0.200f, 0.220f, 0.270f, v ); };
auto CherryText = [] ( float v ) { return ImVec4( 0.860f, 0.930f, 0.890f, v ); };

bool DefaultStyleValid = false;
ImGuiStyle DefaultStyle;

void GenerateThemes()
{
	auto ThemeDefault = []
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		if( !DefaultStyleValid )
		{
			DefaultStyle = Style;
			DefaultStyleValid = true;
		}

		Style = DefaultStyle;
	};

	Themes.insert_or_assign( "ImGui", ThemeDefault );
	ThemeDefault();

	auto ThemeCherry = [ThemeDefault]
	{
		ThemeDefault();

		ImGuiStyle& Style = ImGui::GetStyle();
		Style.Colors[ImGuiCol_Text] = CherryText( 0.78f );
		Style.Colors[ImGuiCol_TextDisabled] = CherryText( 0.28f );
		Style.Colors[ImGuiCol_WindowBg] = ImVec4( 0.13f, 0.14f, 0.17f, 1.00f );
		Style.Colors[ImGuiCol_ChildWindowBg] = CherryBackground( 0.58f );
		Style.Colors[ImGuiCol_PopupBg] = CherryBackground( 0.9f );
		Style.Colors[ImGuiCol_Border] = ImVec4( 0.31f, 0.31f, 1.00f, 0.00f );
		Style.Colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
		Style.Colors[ImGuiCol_FrameBg] = CherryBackground( 1.00f );
		Style.Colors[ImGuiCol_FrameBgHovered] = CherryMedium( 0.78f );
		Style.Colors[ImGuiCol_FrameBgActive] = CherryMedium( 1.00f );
		Style.Colors[ImGuiCol_TitleBg] = CherryLow( 1.00f );
		Style.Colors[ImGuiCol_TitleBgActive] = CherryHigh( 1.00f );
		Style.Colors[ImGuiCol_TitleBgCollapsed] = CherryBackground( 0.75f );
		Style.Colors[ImGuiCol_MenuBarBg] = CherryBackground( 0.47f );
		Style.Colors[ImGuiCol_ScrollbarBg] = CherryBackground( 1.00f );
		Style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.09f, 0.15f, 0.16f, 1.00f );
		Style.Colors[ImGuiCol_ScrollbarGrabHovered] = CherryMedium( 0.78f );
		Style.Colors[ImGuiCol_ScrollbarGrabActive] = CherryMedium( 1.00f );
		Style.Colors[ImGuiCol_CheckMark] = ImVec4( 0.71f, 0.22f, 0.27f, 1.00f );
		Style.Colors[ImGuiCol_SliderGrab] = ImVec4( 0.47f, 0.77f, 0.83f, 0.14f );
		Style.Colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.71f, 0.22f, 0.27f, 1.00f );
		Style.Colors[ImGuiCol_Button] = ImVec4( 0.47f, 0.77f, 0.83f, 0.14f );
		Style.Colors[ImGuiCol_ButtonHovered] = CherryMedium( 0.86f );
		Style.Colors[ImGuiCol_ButtonActive] = CherryMedium( 1.00f );
		Style.Colors[ImGuiCol_Header] = CherryMedium( 0.76f );
		Style.Colors[ImGuiCol_HeaderHovered] = CherryMedium( 0.86f );
		Style.Colors[ImGuiCol_HeaderActive] = CherryHigh( 1.00f );
		Style.Colors[ImGuiCol_Column] = ImVec4( 0.14f, 0.16f, 0.19f, 1.00f );
		Style.Colors[ImGuiCol_ColumnHovered] = CherryMedium( 0.78f );
		Style.Colors[ImGuiCol_ColumnActive] = CherryMedium( 1.00f );
		Style.Colors[ImGuiCol_ResizeGrip] = ImVec4( 0.47f, 0.77f, 0.83f, 0.04f );
		Style.Colors[ImGuiCol_ResizeGripHovered] = CherryMedium( 0.78f );
		Style.Colors[ImGuiCol_ResizeGripActive] = CherryMedium( 1.00f );
		Style.Colors[ImGuiCol_PlotLines] = CherryText( 0.63f );
		Style.Colors[ImGuiCol_PlotLinesHovered] = CherryMedium( 1.00f );
		Style.Colors[ImGuiCol_PlotHistogram] = CherryText( 0.63f );
		Style.Colors[ImGuiCol_PlotHistogramHovered] = CherryMedium( 1.00f );
		Style.Colors[ImGuiCol_TextSelectedBg] = CherryMedium( 0.43f );
		// [...]
		Style.Colors[ImGuiCol_ModalWindowDarkening] = CherryBackground( 0.73f );

		Style.WindowPadding = ImVec2( 6, 4 );
		Style.WindowRounding = 0.0f;
		Style.FramePadding = ImVec2( 5, 2 );
		Style.FrameRounding = 3.0f;
		Style.ItemSpacing = ImVec2( 7, 3 );
		Style.ItemInnerSpacing = ImVec2( 1, 1 );
		Style.TouchExtraPadding = ImVec2( 0, 0 );
		Style.IndentSpacing = 6.0f;
		Style.ScrollbarSize = 12.0f;
		Style.ScrollbarRounding = 16.0f;
		Style.GrabMinSize = 20.0f;
		Style.GrabRounding = 2.0f;

		Style.WindowTitleAlign.x = 0.50f;

		Style.Colors[ImGuiCol_Border] = ImVec4( 0.539f, 0.479f, 0.255f, 0.162f );
	};

	Themes.insert_or_assign( "Cherry", ThemeCherry );

	auto ThemeDracula = [ThemeDefault]
	{
		ThemeDefault();

		ImGuiStyle& Style = ImGui::GetStyle();
		Style.WindowRounding = 5.3f;
		Style.GrabRounding = Style.FrameRounding = 2.3f;
		Style.ScrollbarRounding = 5.0f;
		Style.ItemSpacing.y = 6.5f;

		Style.Colors[ImGuiCol_Text] = { 0.73333335f, 0.73333335f, 0.73333335f, 1.00f };
		Style.Colors[ImGuiCol_TextDisabled] = { 0.34509805f, 0.34509805f, 0.34509805f, 1.00f };
		Style.Colors[ImGuiCol_WindowBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
		Style.Colors[ImGuiCol_PopupBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
		Style.Colors[ImGuiCol_Border] = { 0.33333334f, 0.33333334f, 0.33333334f, 0.50f };
		Style.Colors[ImGuiCol_BorderShadow] = { 0.15686275f, 0.15686275f, 0.15686275f, 0.00f };
		Style.Colors[ImGuiCol_FrameBg] = { 0.16862746f, 0.16862746f, 0.16862746f, 0.54f };
		Style.Colors[ImGuiCol_FrameBgHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
		Style.Colors[ImGuiCol_FrameBgActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
		Style.Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
		Style.Colors[ImGuiCol_TitleBgCollapsed] = { 0.16f, 0.29f, 0.48f, 1.00f };
		Style.Colors[ImGuiCol_TitleBgActive] = { 0.00f, 0.00f, 0.00f, 0.51f };
		Style.Colors[ImGuiCol_MenuBarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.80f };
		Style.Colors[ImGuiCol_ScrollbarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.60f };
		Style.Colors[ImGuiCol_ScrollbarGrab] = { 0.21960786f, 0.30980393f, 0.41960788f, 0.51f };
		Style.Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
		Style.Colors[ImGuiCol_ScrollbarGrabActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 0.91f };
		Style.Colors[ImGuiCol_CheckMark] = { 0.90f, 0.90f, 0.90f, 0.83f };
		Style.Colors[ImGuiCol_SliderGrab] = { 0.70f, 0.70f, 0.70f, 0.62f };
		Style.Colors[ImGuiCol_SliderGrabActive] = { 0.30f, 0.30f, 0.30f, 0.84f };
		Style.Colors[ImGuiCol_Button] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.49f };
		Style.Colors[ImGuiCol_ButtonHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
		Style.Colors[ImGuiCol_ButtonActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 1.00f };
		Style.Colors[ImGuiCol_Header] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.53f };
		Style.Colors[ImGuiCol_HeaderHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
		Style.Colors[ImGuiCol_HeaderActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
		Style.Colors[ImGuiCol_Separator] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
		Style.Colors[ImGuiCol_SeparatorHovered] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
		Style.Colors[ImGuiCol_SeparatorActive] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
		Style.Colors[ImGuiCol_ResizeGrip] = { 1.00f, 1.00f, 1.00f, 0.85f };
		Style.Colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.00f, 1.00f, 0.60f };
		Style.Colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.00f, 0.90f };
		Style.Colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
		Style.Colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 0.43f, 0.35f, 1.00f };
		Style.Colors[ImGuiCol_PlotHistogram] = { 0.90f, 0.70f, 0.00f, 1.00f };
		Style.Colors[ImGuiCol_PlotHistogramHovered] = { 1.00f, 0.60f, 0.00f, 1.00f };
		Style.Colors[ImGuiCol_TextSelectedBg] = { 0.18431373f, 0.39607847f, 0.79215693f, 0.90f };
	};

	Themes.insert_or_assign( "Dracula", ThemeDracula );

	// Default theme
	ThemeCherry();
}

void SetTheme( const char* Theme )
{
	auto Iterator = Themes.find( Theme );
	if( Iterator != Themes.end() )
	{
		Iterator->second();
	}
}

void DebugMenu( CApplication* Application )
{
	if( !Application || !GameLayersInstance )
		return;

	std::vector<IGameLayer*> GameLayers = GameLayersInstance->GetGameLayers();
	IGameLayer* TopLevelLayer = nullptr;

	if( GameLayers.size() > 0 )
	{
		TopLevelLayer = GameLayers[0];
	}

	if( ImGui::BeginMainMenuBar() )
	{
		Version Number;
		
		if( TopLevelLayer )
		{
			Number = TopLevelLayer->GetVersion();
		}

		char MenuName[256];
		sprintf_s( MenuName, "%s %i.%i.%i", Application->GetName().c_str(), Number.Major, Number.Minor, Number.Hot );
		if( ImGui::BeginMenu( MenuName ) )
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

			if( ImGui::MenuItem( "Re-initialize Application", "" ) )
			{
				[Application]
				{
					Application->Initialize();
				}();

				// Has to return when executed because imgui will be reset.
				return;
			}

			if( ImGui::MenuItem( "Reload Configuration", "H" ) )
			{
				InputReloadConfiguration();
			}

			if( ImGui::MenuItem( "Restart Game Layers", "G" ) )
			{
				InputRestartGameLayers();
			}

			if( ImGui::MenuItem( "Reload Shaders", "J" ) )
			{
				InputReloadShaders();
			}

			ImGui::Separator();

			for( auto Iterator : Themes )
			{
				const char* ThemeKey = Iterator.first.c_str();
				char ThemeName[256];
				sprintf_s( ThemeName, "%s Theme", ThemeKey );
				if( ImGui::MenuItem( ThemeName, "" ) )
				{
					SetTheme( ThemeKey );
				}
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

CApplication::CApplication()
{
	Name = "Unnamed Shatter Engine Application";
}

CApplication::~CApplication()
{

}

void CApplication::Run()
{
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
		ZoneScoped;
		MainWindow.BeginFrame();
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
#if defined( IMGUI_ENABLED )
				CProfileVisualisation::GetInstance().Clear();
#endif
				Renderer.RefreshFrame();

				const float TimeScaleParameter = CConfiguration::GetInstance().GetFloat( "timescale" );
				const float TimeScaleGlobal = TimeScale * TimeScaleParameter;
				ScaledGameTime += GameDeltaTime * 0.001f * TimeScaleGlobal;

				// Update time
				GameLayersInstance->Time( ScaledGameTime );
				GameLayersInstance->SetTimeScale( TimeScaleGlobal );

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

#if defined( IMGUI_ENABLED )
			CProfileVisualisation::GetInstance().Display();

			DebugMenu( this );
#endif

			MainWindow.RenderFrame();

			// Tracy profiling marker.
			FrameMark;
		}
	}

	GameLayersInstance->Shutdown();
	delete GameLayersInstance;

	Log::Event( "Meshes created: %i\n", MainWindow.GetRenderer().MeshCount() );

	MainWindow.Terminate();
}

std::string CApplication::GetName() const
{
	return Name;
}

void CApplication::SetName( const char* NameIn )
{
	Name = NameIn;
}

void CApplication::Initialize()
{
	Log::Event( "%s (Build: %s)\n\n", Name.c_str(), __DATE__ );

	ServiceRegistry.CreateStandardServices();

	// Calling GetInstance creates the instance and initializes the class.
	CConfiguration& ConfigurationInstance = CConfiguration::GetInstance();

	char szTitle[256];
	sprintf_s( szTitle, "%s (Build: %s)", Name.c_str(), __DATE__ );

	if( MainWindow.Valid() )
	{
		MainWindow.Terminate();
		ImGui_ImplGlfwGL3_Reset();

		// Reload the configuration file if the application is being re-initialized.
		ConfigurationInstance.Initialize();
	}

	MainWindow.Create( szTitle );

	if( !MainWindow.Valid() )
	{
		Log::Event( Log::Fatal, "Application window could not be created.\n" );
	}

	GLFWwindow* WindowHandle = MainWindow.Handle();
	glfwSetKeyCallback( WindowHandle, InputKeyCallback );
	glfwSetCharCallback( WindowHandle, InputCharCallback );
	glfwSetMouseButtonCallback( WindowHandle, InputMouseButtonCallback );
	glfwSetCursorPosCallback( WindowHandle, InputMousePositionCallback );
	glfwSetScrollCallback( WindowHandle, InputScrollCallback );
	glfwSetJoystickCallback( InputJoystickStatusCallback );

	IInput& Input = CInputLocator::GetService();

	Input.ClearActionBindings();

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

	Input.AddActionBinding( EActionBindingType::Keyboard, GLFW_KEY_ESCAPE, GLFW_RELEASE, []{
		glfwSetWindowShouldClose( MainWindow.Handle(), true );
	} );

	MainWindow.EnableCursor( CursorVisible );

	if( !GameLayersInstance )
	{
		Log::Event( Log::Fatal, "Game layers instance does not exist!\n" );
	}

	if( CameraSpeed < 0.0f )
	{
		CameraSpeed = CConfiguration::GetInstance().GetFloat( "cameraspeed" );
	}

#if defined( IMGUI_ENABLED )
	ImGui_ImplGlfwGL3_Shutdown();

	ImGuiIO& IO = ImGui::GetIO();
	static ImFont* RobotoFont = nullptr;

	ImFontConfig DefaultFontConfig;
	DefaultFontConfig.OversampleH = 4;
	DefaultFontConfig.OversampleV = 2;
	DefaultFontConfig.SizePixels = ConfigurationInstance.GetFloat( "font_size", 15.0f );

	IO.Fonts->ClearTexData();
	RobotoFont = IO.Fonts->AddFontFromFileTTF( "Resources/Roboto-Medium.ttf", DefaultFontConfig.SizePixels, &DefaultFontConfig, IO.Fonts->GetGlyphRangesDefault() );
	IO.FontDefault = RobotoFont;

	ImGui_ImplGlfwGL3_NewFrame();

	GenerateThemes();
#endif
}
