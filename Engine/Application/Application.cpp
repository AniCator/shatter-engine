// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Application.h"

#if defined(_WIN32)
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Audio/SimpleSound.h>
#include <Engine/Display/Window.h>
#include <Engine/Configuration/Configuration.h>

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/MeshBuilder.h>
#include <Engine/Utility/Script/AngelEngine.h>

#include <Game/Game.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.52/imgui.h>
#include <Engine/Display/imgui_impl_glfw_gl3.h>
#endif

CWindow& MainWindow = CWindow::Get();

CCamera DefaultCamera = CCamera();
FCameraSetup& Setup = DefaultCamera.GetCameraSetup();
bool PauseGame = false;
bool ScaleTime = false;
bool CursorVisible = true;
bool RestartLayers = false;

double ScaledGameTime = 0.0;

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
	CConfiguration::Get().Reload();
}

void InputRestartGameLayers(CApplication* Application)
{
	RestartLayers = false;

	if( Application )
	{
		Application->InitializeDefaultInputs();
		Application->UnregisterDebugUI();
	}

	ScaledGameTime = 0.0f;

	// CAngelEngine::Get().Shutdown();
	// CAngelEngine::Get().Initialize();

	GameLayersInstance->Shutdown();

#if defined( IMGUI_ENABLED )
	ImGui_ImplGlfwGL3_Shutdown();
#endif

	GameLayersInstance->Initialize();

	if( Application )
		Application->ResetImGui();

	// Force frame refresh
	MainWindow.GetRenderer().RefreshFrame();
}

void InputReloadShaders()
{
	CAssets::Get().ReloadShaders();
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

#if defined(_DEBUG)
static bool DisplayLog = true;
#else
static bool DisplayLog = false;
#endif
static bool ExportDialog = false;
static size_t PreviousSize = 0;
void DebugMenu( CApplication* Application )
{
	if( !Application || !GameLayersInstance )
		return;

	if( Application->ToolsEnabled() && ImGui::BeginMainMenuBar() )
	{
		Version Number;
		
		std::vector<IGameLayer*> GameLayers = GameLayersInstance->GetGameLayers();
		IGameLayer* TopLevelLayer = nullptr;

		if( GameLayers.size() > 0 )
		{
			TopLevelLayer = GameLayers[0];
		}

		if( TopLevelLayer )
		{
			Number = TopLevelLayer->GetVersion();
		}

		char MenuName[256];
		sprintf_s( MenuName, "%s %i.%i.%i", Application->GetName().c_str(), Number.Major, Number.Minor, Number.Hot );
		if( ImGui::BeginMenu( MenuName ) )
		{
			ImGui::MenuItem( "2017 \xc2\xa9 Christiaan Bakker", nullptr, false, false );

			if( ImGui::MenuItem( "Quit", "Escape" ) )
			{
				glfwSetWindowShouldClose( MainWindow.Handle(), true );
			}

			ImGui::EndMenu();
		}

		if( ImGui::BeginMenu( "Commands" ) )
		{
			if( ImGui::MenuItem( "Pause", nullptr, PauseGame ) )
			{
				PauseGame = !PauseGame;
			}

			if( ImGui::MenuItem( "Slow Motion", nullptr, ScaleTime ) )
			{
				ScaleTime = !ScaleTime;
			}

			ImGui::Separator();

			if( ImGui::MenuItem( "Re-initialize Application", "" ) )
			{
				Application->Initialize();

				// Has to return when executed because imgui will be reset.
				return;
			}

			if( ImGui::MenuItem( "Reload Configuration", "H" ) )
			{
				InputReloadConfiguration();
			}

			if( ImGui::MenuItem( "Restart Game Layers", "G" ) )
			{
				RestartLayers = true;
			}

			if( ImGui::MenuItem( "Reload Shaders", "J" ) )
			{
				InputReloadShaders();
			}

			const bool Enabled = MainWindow.GetRenderer().ForceWireFrame;
			if( ImGui::MenuItem( "Toggle Wireframe", nullptr, Enabled ) )
			{
				MainWindow.GetRenderer().ForceWireFrame = !Enabled;
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

		if( Application->DebugFunctions() > 0 )
		{
			if( ImGui::BeginMenu( "Game" ) )
			{
				Application->RenderDebugUI( true );
				ImGui::EndMenu();
			}
		}

		if( ImGui::BeginMenu( "Windows" ) )
		{
			CProfiler& Profiler = CProfiler::Get();
			const bool Enabled = Profiler.IsEnabled();
			if( ImGui::MenuItem( "Profiler", nullptr, Enabled ) )
			{
				Profiler.SetEnabled( !Enabled );
			}

			if( ImGui::MenuItem( "Logger", nullptr, DisplayLog ) )
			{
				DisplayLog = !DisplayLog;
			}

			if( ImGui::MenuItem( "Export" ) )
			{
				ExportDialog = true;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if( DisplayLog )
	{
		const float Width = static_cast<float>( MainWindow.GetWidth() );
		const float Height = 0.2f * static_cast<float>( MainWindow.GetHeight() );

		ImGui::SetNextWindowPos( ImVec2( 0.0f, MainWindow.GetHeight() - Height ), ImGuiCond_Always );
		ImGui::SetNextWindowSize( ImVec2( Width, Height ), ImGuiCond_Always );

		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.3f ) ); // Transparent background
		if( ImGui::Begin( "Log", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
		{
			static const char* SeverityToString[Log::LogMax] =
			{
				"",
				"Warning",
				"Error",
				"Fatal"
			};

			static const ImVec4 SeverityToColor[Log::LogMax] =
			{
				ImVec4( 1.0f,1.0f,1.0f,1.0f ),
				ImVec4( 1.0f,1.0f,0.0f,1.0f ),
				ImVec4( 1.0f,0.0f,0.0f,1.0f ),
				ImVec4( 1.0f,0.0f,0.0f,1.0f ),
			};

			const std::vector<Log::FHistory>& LogHistory = Log::History();
			const size_t Count = LogHistory.size();
			const size_t Entries = LogHistory.size() < 500 ? LogHistory.size() : 500;
			for( size_t Index = 0; Index < Entries; Index++ )
			{
				const size_t HistoryIndex = Count - Entries + Index;
				const Log::FHistory& History = LogHistory[HistoryIndex];
				if( History.Severity > Log::Standard )
				{
					ImGui::TextColored( SeverityToColor[History.Severity], "%s: %s", SeverityToString[History.Severity], History.Message.c_str() );
				}
				else
				{
					ImGui::Text( "%s", History.Message.c_str() );
				}
			}

			if( PreviousSize != Count )
			{
				ImGui::SetScrollHere( 1.0f );
				PreviousSize = Count;
			}
		}
		ImGui::End();

		ImGui::PopStyleColor();
	}

	if( ExportDialog )
	{
		if( ImGui::Begin( "Export", &ExportDialog, ImVec2( 600.0f, 800.0f ) ) )
		{
			ImGui::Text( "Click on an asset to convert it." );

			auto Map = CAssets::Get().GetMeshes();
			for( auto Entry : Map )
			{
				if( ImGui::Button( Entry.first.c_str() ) )
				{
					Log::Event( "Exporting mesh \"%s\".", Entry.first.c_str() );
					CMesh* Mesh = Entry.second;
					if( Mesh )
					{
						FPrimitive Primitive;
						MeshBuilder::Mesh( Primitive, Mesh );

						CData Data;
						Data << Primitive;

						std::stringstream Location;
						Location << "Models/" << Entry.first << ".lm";

						CFile File( Location.str().c_str() );
						File.Load( Data );
						File.Save();
					}
				}
			}

			ImGui::Text( "Import speed test." );

			if( ImGui::Button( "Import Island OBJ" ) )
			{
				CTimer LoadTimer;
				CTimer ParseTimer;

				CFile File( "Models/island.obj" );

				LoadTimer.Start();
				File.Load();
				LoadTimer.Stop();

				FPrimitive Primitive;

				ParseTimer.Start();
				MeshBuilder::OBJ( Primitive, File );
				ParseTimer.Stop();

				Log::Event( "Import speed test: Load %ims Parse %ims\n", LoadTimer.GetElapsedTimeMilliseconds(), ParseTimer.GetElapsedTimeMilliseconds() );
			}

			ImGui::Text( "Re-import" );

			ImGui::Columns( 3 );
			static char AssetName[128];
			ImGui::InputText( "Asset name", AssetName, 128 );
			ImGui::NextColumn();

			static char AssetPath[512];
			ImGui::InputText( "Asset path", AssetPath, 512 );
			ImGui::NextColumn();

			if( ImGui::Button( "Re-Import" ) )
			{
				auto& Assets = CAssets::Get();
				if( Assets.FindMesh( AssetName ) )
				{
					CTimer LoadTimer;

					LoadTimer.Start();
					auto Mesh = Assets.CreateNamedMesh( AssetName, AssetPath, true );
					LoadTimer.Stop();

					const size_t Triangles = Mesh->GetVertexBufferData().IndexCount / 3;

					Log::Event( "Re-import: %ims %i triangles\n", LoadTimer.GetElapsedTimeMilliseconds(), Triangles );
				}
			}
			ImGui::NextColumn();

			ImGui::Columns();
		}

		ImGui::End();
	}

	Application->RenderDebugUI( false );
}

CApplication::CApplication()
{
	Name = "Unnamed Shatter Engine Application";
	Tools = false;
	DefaultExit = true;
}

CApplication::~CApplication()
{

}

void CApplication::Run()
{
	CRenderer& Renderer = MainWindow.GetRenderer();

	Initialize();

	Setup.AspectRatio = static_cast<float>( MainWindow.GetWidth() ) / static_cast<float>( MainWindow.GetHeight() );
	Renderer.SetCamera( DefaultCamera );

	CTimer InputTimer( false );
	CTimer GameTimer( false );
	CTimer RenderTimer( false );

	InputTimer.Start();
	GameTimer.Start();
	RenderTimer.Start();

	const int FPSLimit = CConfiguration::Get().GetInteger( "fps", 300 );

	const uint64_t MaximumGameTime = 1000 / CConfiguration::Get().GetInteger( "tickrate", 60 );
	const uint64_t MaximumInputTime = 1000 / CConfiguration::Get().GetInteger( "pollingrate", 120 );
	const uint64_t MaximumFrameTime = 1000 / CConfiguration::Get().GetInteger( "fps", 60 );

	while( !MainWindow.ShouldClose() )
	{
		if( RestartLayers )
			InputRestartGameLayers( this );

		const uint64_t RenderDeltaTime = RenderTimer.GetElapsedTimeMilliseconds();
		if( RenderDeltaTime >= MaximumFrameTime || FPSLimit < 1 )
		{
			CTimerScope Scope_( "Frametime", false );
			MainWindow.BeginFrame();

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
					CProfiler::Get().Clear();
					Renderer.RefreshFrame();

					const float TimeScaleParameter = CConfiguration::Get().GetFloat( "timescale", 1.0f );
					const float TimeScaleGlobal = TimeScale * TimeScaleParameter;
					ScaledGameTime += GameDeltaTime * 0.001f * TimeScaleGlobal;

					// Update time
					GameLayersInstance->Time( ScaledGameTime );
					GameLayersInstance->SetTimeScale( TimeScaleGlobal );

					// Tick all game layers
					GameLayersInstance->Tick();

					// CAngelEngine::Get().Tick();

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
				CProfiler::Get().Display();
				CProfiler::Get().ClearFrame();
				DebugMenu( this );
#endif

				MainWindow.RenderFrame();
				RenderTimer.Start();
			}
		}
	}

	// CAngelEngine::Get().Shutdown();

	GameLayersInstance->Shutdown();
	delete GameLayersInstance;

	MainWindow.Terminate();

	CSimpleSound::Shutdown();
}

void CApplication::Close()
{
	glfwSetWindowShouldClose( MainWindow.Handle(), true );
}

void CApplication::InitializeDefaultInputs()
{
	IInput& Input = CInputLocator::GetService();

	Input.ClearActionBindings();

	Input.AddActionBinding( EKey::Space, EAction::Press, InputPauseGameEnable );
	Input.AddActionBinding( EKey::Space, EAction::Release, InputPauseGameDisable );

	Input.AddActionBinding( EKey::Enter, EAction::Press, InputScaleTimeEnable );
	Input.AddActionBinding( EKey::Enter, EAction::Release, InputScaleTimeDisable );

	Input.AddActionBinding( EKey::H, EAction::Release, InputReloadConfiguration );
	Input.AddActionBinding( EKey::G, EAction::Release, [] () {
		RestartLayers = true;
	} );
	Input.AddActionBinding( EKey::J, EAction::Release, InputReloadShaders );

	Input.AddActionBinding( EKey::W, EAction::Press, InputMoveCameraUp );
	Input.AddActionBinding( EKey::S, EAction::Press, InputMoveCameraDown );
	Input.AddActionBinding( EKey::A, EAction::Press, InputMoveCameraLeft );
	Input.AddActionBinding( EKey::D, EAction::Press, InputMoveCameraRight );
	Input.AddActionBinding( EKey::R, EAction::Press, InputMoveCameraLower );
	Input.AddActionBinding( EKey::F, EAction::Press, InputMoveCameraHigher );

	if( DefaultExit )
	{
		Input.AddActionBinding( EKey::Escape, EAction::Release, [this] {
			Close();
		} );
	}

	Input.AddActionBinding( EKey::L, EAction::Release, [] {
		DisplayLog = !DisplayLog;
	} );

	Input.AddActionBinding( EKey::P, EAction::Release, [] {
		CProfiler& Profiler = CProfiler::Get();
		Profiler.SetEnabled( !Profiler.IsEnabled() );
	} );

	Input.AddActionBinding( EKey::NumpadSubtract, EAction::Release, [this] {
		Tools = !Tools;
	} );
}

void CApplication::ResetImGui()
{
#if defined( IMGUI_ENABLED )
	Log::Event( "Setting font configuration.\n" );
	static const char* FontLocation = "Resources/Roboto-Medium.ttf";
	const bool FileExists = CFile::Exists( FontLocation );
	if( FileExists )
	{
		ImGuiIO& IO = ImGui::GetIO();
		static ImFont* RobotoFont = nullptr;

		ImFontConfig DefaultFontConfig;
		DefaultFontConfig.OversampleH = 4;
		DefaultFontConfig.OversampleV = 2;
		DefaultFontConfig.SizePixels = CConfiguration::Get().GetFloat( "font_size", 15.0f );

		RobotoFont = IO.Fonts->AddFontFromFileTTF( FontLocation, DefaultFontConfig.SizePixels, &DefaultFontConfig, IO.Fonts->GetGlyphRangesDefault() );
		IO.FontDefault = RobotoFont;
	}
	else
	{
		Log::Event( Log::Warning, "Could not find a default resource.\nMake sure you include the default engine items in the build directory.\n" );
	}

	ImGui_ImplGlfwGL3_NewFrame();

	GenerateThemes();
#endif
}

std::string CApplication::GetName() const
{
	return Name;
}

void CApplication::SetName( const char* NameIn )
{
	Name = NameIn;
}

void CApplication::RedirectLogToConsole()
{
#if defined(_WIN32)
	if( FreeConsole() && AttachConsole( ATTACH_PARENT_PROCESS ) )
	{
		Log::CLog::Get().ToStdOut = true;

		FILE* fp;
		freopen_s( &fp, "CONOUT$", "w", stdout );

		std::ios::sync_with_stdio();
	}
#endif
}

void CApplication::ProcessCommandLine( int argc, char ** argv )
{
	for( int Index = 0; Index < argc; Index++ )
	{
		if( strcmp( argv[Index], "-tools" ) == 0 )
		{
			EnableTools( true );
		}
		else if( strcmp( argv[Index], "-convert" ) == 0 )
		{
			RedirectLogToConsole();

			if( Index + 1 < argc )
			{			
				CTimer LoadTimer;
				CTimer ParseTimer;

				const char* Location = argv[Index + 1];
				CFile File( Location );
				if( File.Extension() == "obj" )
				{
					LoadTimer.Start();
					File.Load();
					LoadTimer.Stop();

					FPrimitive Primitive;

					ParseTimer.Start();
					MeshBuilder::OBJ( Primitive, File );
					ParseTimer.Stop();

					CData Data;
					Data << Primitive;

					std::string ExportPath = Location;
					ExportPath = ExportPath.substr( 0, ExportPath.length() - 3 );
					ExportPath.append( "lm" );

					CFile File( ExportPath.c_str() );
					File.Load( Data );
					File.Save();

					Log::Event( "OBJ Import | Load %ims Parse %ims\n", LoadTimer.GetElapsedTimeMilliseconds(), ParseTimer.GetElapsedTimeMilliseconds() );
				}

				exit( 1337 );
			}
		}
	}
}

const bool CApplication::ToolsEnabled() const
{
	return Tools;
}

void CApplication::EnableTools( const bool Enable )
{
	Tools = Enable;
}

const bool CApplication::DefaultExitEnabled() const
{
	return DefaultExit;
}

void CApplication::EnableDefaultExit( const bool Enable )
{
	DefaultExit = Enable;
}

void CApplication::RegisterDebugUI( DebugUIFunction Function )
{
	DebugUIFunctions.push_back( Function );
}

void CApplication::RenderDebugUI( const bool Menu )
{
	for( auto& DebugUIFunction : DebugUIFunctions )
	{
		DebugUIFunction( Menu );
	}
}

void CApplication::UnregisterDebugUI()
{
	DebugUIFunctions.clear();
}

const glm::size_t CApplication::DebugFunctions() const
{
	return DebugUIFunctions.size();
}

#if defined(_WIN32)
#include "Windows.h"

LONG WINAPI ExceptionHandler( EXCEPTION_POINTERS* ExceptionInfo )
{
	const std::vector<Log::FHistory>& LogHistory = Log::History();
	if( LogHistory.size() > 0 )
	{
		Log::Event( Log::Fatal, "An exception has occured. Guess I'll die.\n\nLast log message:\n%s\n", LogHistory[LogHistory.size() - 1].Message.c_str() );
	}
	else
	{
		Log::Event( Log::Fatal, "An exception has occured. Guess I'll die.\n" );
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif

void CApplication::Initialize()
{
#if defined(_WIN32)
	// Attach an exception handler.
	::SetUnhandledExceptionFilter( ExceptionHandler );
#endif

	Log::Event( "%s (Build: %s)\n\n", Name.c_str(), __DATE__ );

#if defined( IMGUI_ENABLED )
	ImGui_ImplGlfwGL3_Shutdown();
#endif

	unsigned char EndianTest[2] = { 1, 0 };
	short CastShort;

	CastShort = *reinterpret_cast<short*>( EndianTest );
	if( CastShort == 0 )
	{
		Log::Event( "Byte order: Little endian.\n" );
	}
	else
	{
		Log::Event( "Byte order: Big endian.\n" );
	}

	ServiceRegistry.CreateStandardServices();

	// Calling Get creates the instance and initializes the class.
	CConfiguration& ConfigurationInstance = CConfiguration::Get();

	if( MainWindow.Valid() )
	{
		MainWindow.Terminate();
		ImGui_ImplGlfwGL3_Reset();

		// Reload the configuration file if the application is being re-initialized.
		ConfigurationInstance.Initialize();
	}

	MainWindow.Create( Name.c_str() );

	if( !MainWindow.Valid() )
	{
		Log::Event( Log::Fatal, "Application window could not be created.\n" );
	}

	ResetImGui();

	// Render a single frame to indicate we're initializing.
	if( !MainWindow.ShouldClose() )
	{
		MainWindow.BeginFrame();

#if defined( IMGUI_ENABLED )
		ImGui::SetNextWindowPos( ImVec2( 0.0f, 0.0f ), ImGuiCond_Always );
		ImGui::SetNextWindowSize( ImVec2( 500.0f, 20.0f ), ImGuiCond_Always );

		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.3f ) ); // Transparent background
		if( ImGui::Begin( "Loading", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
		{
			ImGui::Text( "Loading..." );
			ImGui::End();
		}

		ImGui::PopStyleColor();
#endif

		MainWindow.RenderFrame();
	}

#if defined( IMGUI_ENABLED )
	ImGui_ImplGlfwGL3_Shutdown();
#endif

	Log::Event( "Binding engine inputs.\n" );

	GLFWwindow* WindowHandle = MainWindow.Handle();
	glfwSetKeyCallback( WindowHandle, InputKeyCallback );
	glfwSetCharCallback( WindowHandle, InputCharCallback );
	glfwSetMouseButtonCallback( WindowHandle, InputMouseButtonCallback );
	glfwSetCursorPosCallback( WindowHandle, InputMousePositionCallback );
	glfwSetScrollCallback( WindowHandle, InputScrollCallback );
	glfwSetJoystickCallback( InputJoystickStatusCallback );

	InitializeDefaultInputs();

	MainWindow.EnableCursor( CursorVisible );

	if( !GameLayersInstance )
	{
		Log::Event( Log::Fatal, "Game layers instance does not exist!\n" );
	}

	if( CameraSpeed < 0.0f )
	{
		CameraSpeed = CConfiguration::Get().GetFloat( "cameraspeed", 1.0f );
	}

	UnregisterDebugUI();

	// CAngelEngine::Get().Initialize();

	GameLayersInstance->Initialize();

	ResetImGui();
}
