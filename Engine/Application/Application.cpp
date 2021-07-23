// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Application.h"
#include "ApplicationMenu.h"

#if defined(_WIN32)
#include <Windows.h>
// #include <fcntl.h>
// #include <io.h>

#include <dbghelp.h>
// #include <shellapi.h>
#include <shlobj.h>

// #define WINVER 0x0600
// #define _WIN32_WINNT 0x0600

#include <stdio.h>
#include <objbase.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#endif

#include <filesystem>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Configuration/Configuration.h>

#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/MeshBuilder.h>
#include <Engine/Utility/Script/AngelEngine.h>
#include <Engine/World/World.h>

#include <Game/Game.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.70/imgui.h>
#include <Engine/Display/imgui_impl_glfw.h>
#include <Engine/Display/imgui_impl_opengl3.h>
#endif

CWindow& MainWindow = CWindow::Get();

std::string CApplication::Name = "Unnamed Shatter Engine Application";
std::wstring CApplication::DirectoryName = L"UnnamedShatterGame";

CCamera DefaultCamera = CCamera();
FCameraSetup& Setup = DefaultCamera.GetCameraSetup();
static bool PauseGame = false;
bool FrameStep = false;
bool ScaleTime = false;
bool CursorVisible = true;
bool RestartLayers = false;

double ScaledGameTime = 0.0;

void InputScaleTimeEnable( const float& Scale = 1.0f )
{
	ScaleTime = true;
}

void InputScaleTimeDisable( const float& Scale = 1.0f )
{
	ScaleTime = false;
}

void InputPauseGameEnable( const float& Scale = 1.0f )
{
	PauseGame = true;
}

void InputPauseGameDisable( const float& Scale = 1.0f )
{
	PauseGame = !PauseGame;
}

void InputReloadConfiguration( const float& Scale = 1.0f )
{
	CConfiguration::Get().Reload();
}

void InputRestartGameLayers(CApplication* Application)
{
	RestartLayers = false;

	MainWindow.RenderFrame();

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
	ImGui_ImplGlfw_Shutdown();
#endif

	GameLayersInstance->Initialize();

	if( Application )
		Application->ResetImGui();

	// Force frame refresh
	MainWindow.GetRenderer().RefreshFrame();
}

void InputReloadShaders( const float& Scale = 1.0f )
{
	CAssets::Get().ReloadShaders();
}

float CameraSpeed = -1.0f;
void InputMoveCameraUp( const float& Scale = 1.0f )
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[1] += Speed;
}

void InputMoveCameraDown( const float& Scale = 1.0f )
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[1] -= Speed;
}

void InputMoveCameraLeft( const float& Scale = 1.0f )
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[0] -= Speed;
}

void InputMoveCameraRight( const float& Scale = 1.0f )
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[0] += Speed;
}

void InputMoveCameraLower( const float& Scale = 1.0f )
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[2] -= Speed;
}

void InputMoveCameraHigher( const float& Scale = 1.0f )
{
	const float DistanceToZero = fabs( Setup.CameraPosition[2] );
	const float Speed = CameraSpeed * DistanceToZero;

	Setup.CameraPosition[2] += Speed;
}

void InputToggleMouse( const float& Scale = 1.0f )
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

void SetTheme( const std::string& Theme )
{
	const auto& Iterator = Themes.find( Theme );
	if( Iterator != Themes.end() )
	{
		Iterator->second();
	}
}

static bool ShowTestWindow = false;
static bool ShowMetricsWindow = false;

static bool DisplayLog = false;
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

			if( ImGui::MenuItem( "Frame Step", nullptr, FrameStep ) )
			{
				FrameStep = true;
			}

			if( ImGui::MenuItem( "Slow Motion", nullptr, ScaleTime ) )
			{
				ScaleTime = !ScaleTime;
			}

			ImGui::Separator();

			//if( ImGui::MenuItem( "Re-initialize Application", "" ) )
			//{
			//	Application->Initialize();

			//	// Has to return when executed because imgui will be reset.
			//	return;
			//}

			if( ImGui::MenuItem( "800x600", "" ) )
			{
				MainWindow.Resize( ViewDimensions( 800, 600 ) );
			}

			if( ImGui::MenuItem( "1280x720", "" ) )
			{
				MainWindow.Resize( ViewDimensions( 1280, 720 ) );
			}

			if( ImGui::MenuItem( "1920x1080", "" ) )
			{
				MainWindow.Resize( ViewDimensions( 1920, 1080 ) );
			}

			if( ImGui::MenuItem( "2560x1440", "" ) )
			{
				MainWindow.Resize( ViewDimensions( 2560, 1440 ) );
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

			for( const auto& Iterator : Themes )
			{
				const auto& ThemeKey = Iterator.first;
				const std::string ThemeName = ThemeKey + " Theme";
				if( ImGui::MenuItem( ThemeName.c_str() ) )
				{
					SetTheme( ThemeKey );
				}
			}

			ImGui::EndMenu();
		}

		if( Application->DebugFunctions() > 0 )
		{
			Application->RenderDebugUI( true );
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

			ImGui::Separator();

			RenderMenuItems();

			MenuItem( "ImGui Test Window", &ShowTestWindow );
			MenuItem( "ImGui Metrics Window", &ShowMetricsWindow );

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

			const auto& LogHistory = Log::History();
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

	RenderMenuPanels();

	if( ShowTestWindow )
	{
		ImGui::ShowDemoWindow( &ShowTestWindow );
	}

	if( ShowMetricsWindow )
	{
		ImGui::ShowMetricsWindow( &ShowMetricsWindow );
	}

	Application->RenderDebugUI( false );
}

CApplication::CApplication()
{
	Tools = false;
	DefaultExit = true;
	WaitForInput = false;
}

CApplication::~CApplication()
{

}

std::string CApplication::UTF16ToUTF8( const std::wstring& UTF16 )
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> Converter;
	return Converter.to_bytes( UTF16 );
}

std::wstring CApplication::UTF8ToUTF16( const std::string& UTF8 )
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> Converter;
	return Converter.from_bytes( UTF8 );
}

std::wstring CApplication::Relative( const std::wstring& UTF16 )
{
	std::wstring Path = UTF16;

	const auto ApplicationDirectoryWide = CApplication::GetApplicationDirectory();

	size_t Position = std::string::npos;
	while( ( Position = Path.find( ApplicationDirectoryWide ) ) != std::string::npos )
	{
		Path.erase( Position, ApplicationDirectoryWide.length() );
	}

	std::replace( Path.begin(), Path.end(), '\\', '/' );

	return Path;
}

std::string CApplication::Relative( const std::string& UTF8 )
{
	std::string Path = UTF8;
	
	const auto ApplicationDirectoryWide = CApplication::GetApplicationDirectory();
	const auto ApplicationDirectory = CApplication::UTF16ToUTF8( ApplicationDirectoryWide ) + "\\";

	size_t Position = std::string::npos;
	while( ( Position = Path.find( ApplicationDirectory ) ) != std::string::npos )
	{
		Path.erase( Position, ApplicationDirectory.length() );
	}

	std::replace( Path.begin(), Path.end(), '\\', '/' );

	return Path;
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

	CTimer RealTime( false );
	RealTime.Start();

	SetFPSLimit( CConfiguration::Get().GetInteger( "fps", 300 ) );

	const uint64_t MaximumGameTime = 1000 / CConfiguration::Get().GetInteger( "tickrate", 60 );
	const uint64_t MaximumInputTime = 1000 / CConfiguration::Get().GetInteger( "pollingrate", 120 );

	const float GlobalVolume = CConfiguration::Get().GetFloat( "volume", 100.0f );
	CSoLoudSound::Volume( GlobalVolume );

	while( !MainWindow.ShouldClose() )
	{
		if( RestartLayers )
			InputRestartGameLayers( this );

		const uint64_t MaximumFrameTime = FPSLimit > 0 ? 1000 / FPSLimit : 999;
		const uint64_t RenderDeltaTime = RenderTimer.GetElapsedTimeMilliseconds();
		if( RenderDeltaTime >= MaximumFrameTime || FPSLimit < 1 )
		{
			CTimerScope Scope_( "Frametime", RenderDeltaTime );

			GameLayersInstance->FrameTime( StaticCast<double>( RealTime.GetElapsedTimeMilliseconds() ) * 0.001 );

			const uint64_t InputDeltaTime = InputTimer.GetElapsedTimeMilliseconds();
			if( InputDeltaTime >= MaximumInputTime )
			{
				if( !MainWindow.IsWindowless() )
				{
					SetMouseWheel( ImGui::GetIO().MouseWheel );
				}

				MainWindow.ProcessInput();

				InputTimer.Start();
			}

			MainWindow.BeginFrame();

			const float TimeScale = ScaleTime ? 0.01f : 1.0f;

			const uint64_t GameDeltaTime = GameTimer.GetElapsedTimeMilliseconds();
			if( GameDeltaTime >= MaximumGameTime )
			{
				if( !Tools )
				{
					if( MainWindow.IsCursorEnabled() )
					{
						MainWindow.EnableCursor( false );
					}
				}
				else
				{
					if( !MainWindow.IsCursorEnabled() )
					{
						MainWindow.EnableCursor( true );
					}
				}

				if( !PauseGame || FrameStep )
				{
					CProfiler::Get().Clear();
					Renderer.RefreshFrame();

					const double TimeScaleParameter = CConfiguration::Get().GetDouble( "timescale", 1.0 );
					const double TimeScaleGlobal = TimeScale * TimeScaleParameter;

					// Clamp the maximum time added to avoid excessive tick spikes.
					ScaledGameTime += Math::Clamp( GameDeltaTime * 0.001 * TimeScaleGlobal, 0.0, 1.0 );

					// Update game time.
					GameLayersInstance->Time( ScaledGameTime );
					GameLayersInstance->SetTimeScale( TimeScaleGlobal );

					// Update the renderable stage for tick functions.
					Renderer.UpdateRenderableStage( RenderableStage::Tick );
					
					// Tick all game layers
					GameLayersInstance->Tick();

					GameTimer.Start();

					if( FrameStep )
					{
						FrameStep = false;
					}
				}
			}

			if( PauseGame && !FrameStep )
			{
				GameTimer.Start();
			}

			{
				Renderer.UpdateRenderableStage( RenderableStage::Frame );
				GameLayersInstance->Frame();

#if defined( IMGUI_ENABLED )
				if( !MainWindow.IsWindowless() )
				{
					CProfiler::Get().Display();
					CProfiler::Get().ClearFrame();
					DebugMenu( this );
				}
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

	CSoLoudSound::Shutdown();
}

void CApplication::Close()
{
	glfwSetWindowShouldClose( MainWindow.Handle(), true );
}

void CApplication::InitializeDefaultInputs()
{
	IInput& Input = CInputLocator::Get();

	Input.ClearActionBindings();

	Input.AddActionBinding( "TimeScaleEnable", EKey::Enter, EAction::Press, InputScaleTimeEnable );
	Input.AddActionBinding( "TimeScaleDisable", EKey::Enter, EAction::Release, InputScaleTimeDisable );

	Input.AddActionBinding( "ReloadConfiguration", EKey::H, EAction::Release, InputReloadConfiguration );
	Input.AddActionBinding( "RestartGameLayers", EKey::G, EAction::Release, [] ( const float& Scale ) {
		RestartLayers = true;
	} );
	Input.AddActionBinding( "ReloadShaders", EKey::J, EAction::Release, InputReloadShaders );

	Input.AddActionBinding( "CameraUp", EKey::W, EAction::Press, InputMoveCameraUp );
	Input.AddActionBinding( "CameraDown", EKey::S, EAction::Press, InputMoveCameraDown );
	Input.AddActionBinding( "CameraLeft", EKey::A, EAction::Press, InputMoveCameraLeft );
	Input.AddActionBinding( "CameraRight", EKey::D, EAction::Press, InputMoveCameraRight );
	Input.AddActionBinding( "CameraLower", EKey::R, EAction::Press, InputMoveCameraLower );
	Input.AddActionBinding( "CameraHigher", EKey::F, EAction::Press, InputMoveCameraHigher );

	if( DefaultExit )
	{
		Input.AddActionBinding( "Exit", EKey::Escape, EAction::Release, [&] ( const float& Scale ) {
			Close();
		} );
	}

	Input.AddActionBinding( "DisplayLog", EKey::L, EAction::Release, [] ( const float& Scale ) {
		DisplayLog = !DisplayLog;
	} );

	Input.AddActionBinding( "FrameStep", EKey::F5, EAction::Release, [] ( const float& Scale ) {
		FrameStep = true;
		} );

	Input.AddActionBinding( "Pause", EKey::F6, EAction::Release, [] ( const float& Scale ) {
		PauseGame = !PauseGame;
		} );

	Input.AddActionBinding( "ShowToolbar", EKey::NumpadSubtract, EAction::Release, [&] ( const float& Scale ) {
		Tools = !Tools;
	} );

	Input.AddActionBinding( "ShowProfiler", EKey::NumpadAdd, EAction::Release, [] ( const float& Scale ) {
		CProfiler& Profiler = CProfiler::Get();
		Profiler.SetEnabled( !Profiler.IsEnabled() );
	} );
}

void CApplication::ResetImGui()
{
	if( MainWindow.IsWindowless() )
		return;

#if defined( IMGUI_ENABLED )
	ImGuiIO& IO = ImGui::GetIO();
	IO.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

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

	while( !ImGui::GetIO().Fonts->IsBuilt() )
	{
		Log::Event( "Building fonts.\n" );
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplOpenGL3_NewFrame();
	}

	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	UI::Reset();

	GenerateThemes();

	ImGui::Render();
#endif
}

const std::string& CApplication::GetName()
{
	return Name;
}

void CApplication::SetName( const char* NameIn )
{
	Name = NameIn;
}

const std::wstring& CApplication::GetDirectoryName()
{
	if( DirectoryName.length() == 0 )
	{
		static const std::wstring UnnamedProject = L"UnnamedShatterGame";
		return UnnamedProject;
	}
	
	return DirectoryName;
}

void CApplication::SetDirectoryName( const wchar_t* Name )
{
	DirectoryName = Name;
}

const std::wstring& CApplication::GetAppDataDirectory()
{
	static std::wstring AppDataDirectory;
	if( AppDataDirectory.empty() )
	{
		wchar_t* UserPath = nullptr;

		HRESULT Ret = SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, NULL, &UserPath );
		if( SUCCEEDED( Ret ) )
		{
			std::wstringstream Location;
			Location << UserPath;

			AppDataDirectory = Location.str();
			CoTaskMemFree( UserPath );
		}
	}

	return AppDataDirectory;
}

const std::wstring& CApplication::GetUserSettingsDirectory()
{
	static const std::wstring Separator = L"/";
	static const std::wstring Directory = GetDirectoryName() + Separator;
	static const std::wstring AppData = GetAppDataDirectory() + Separator;
	static const std::wstring UserSettingsDirectory = AppData + Directory;
	return UserSettingsDirectory;
}

const std::wstring& CApplication::GetUserSettingsFileName()
{
	static const std::wstring FileName = L"Configuration.ini";
	return FileName;
}

const std::wstring& CApplication::GetUserSettingsPath()
{
	static const std::wstring Path = GetUserSettingsDirectory() + GetUserSettingsFileName();
	return Path;
}

const std::wstring& CApplication::GetApplicationDirectory()
{
	static const std::wstring Path = std::experimental::filesystem::canonical( std::experimental::filesystem::path() );
	return Path;
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
	CommandLine.clear();

	for( int Index = 0; Index < argc; Index++ )
	{
		bool Command = false;
		if( argv[Index][0] == '-' )
		{
			Command = true;
		}

		if( Command )
		{
			const size_t NextIndex = Index + 1;
			if( NextIndex < argc && argv[NextIndex][0] != '-' )
			{
				CommandLine.insert_or_assign( argv[Index], argv[NextIndex] );
			}
			else
			{
				CommandLine.insert_or_assign( argv[Index], "" );
			}
		}

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
		else if( strcmp( argv[Index], "-waitforinput" ) == 0 )
		{
			WaitForInput = true;
		}
		else if( strcmp( argv[Index], "-server" ) == 0 )
		{
			MainWindow.SetWindowless( true );
		}
		else if( strcmp( argv[Index], "-atlas" ) == 0 )
		{
			RedirectLogToConsole();

			MainWindow.Create( "Atlas" );
			MainWindow.Resize( ViewDimensions( 64, 64 ) );

			Log::Event( "Generating atlas...\n" );

			std::deque<CTexture> Textures;

			int NextIndex = Index + 1;
			while( NextIndex < argc )
			{
				auto Location = argv[NextIndex++];
				Log::Event( "Path: %s\n", Location );
				Textures.emplace_back( CTexture( Location ) );
				Textures[Textures.size() - 1].Load();

			}

			if( Textures.size() > 0 )
			{
				int SourceIndex = 0;
				int SampleIndex = 0;
				bool SourceValid = true;
				auto ImageData = Textures[SourceIndex].GetImageData();

				const int AtlasSize = 1024;
				const int AtlasChannels = 3;
				const size_t AtlasLength = AtlasSize * AtlasSize * AtlasChannels;
				unsigned char* AtlasData = new unsigned char[AtlasLength];

				unsigned char OddColor[AtlasChannels] = {
					255,0,255
				};

				unsigned char EvenColor[AtlasChannels] = {
					0,0,0
				};

				bool Odd = false;
				for( size_t Index = 0; Index < AtlasLength; Index++ )
				{
					size_t Offset = Index % ( AtlasSize * AtlasChannels );
					const size_t Channel = ( Index ) % AtlasChannels;
					if( Channel == 0 )
					{
						Odd = !Odd;
					}

					if( Offset == 0 )
					{
						Odd = !Odd;
					}

					auto Pointer = reinterpret_cast<unsigned char*>( ImageData );
					if( SourceValid && !Pointer[SampleIndex] )
					{
						SourceValid = false;
						SourceIndex++;
						if( SourceIndex < Textures.size() )
						{
							ImageData = Textures[SourceIndex].GetImageData();
							SampleIndex = 0;

							Pointer = reinterpret_cast<unsigned char*>( ImageData );
							if( Pointer[SampleIndex] )
							{
								SourceValid = true;
							}
						}
					}

					if( SourceValid )
					{
						AtlasData[Index] = Pointer[SampleIndex];
					}
					else
					{
						if( Odd )
						{
							AtlasData[Index] = OddColor[Channel];
						}
						else
						{
							AtlasData[Index] = EvenColor[Channel];
						}
					}

					SampleIndex++;
				}

				CTexture Atlas;
				Atlas.Load( AtlasData, AtlasSize, AtlasSize, AtlasChannels );
				Atlas.Save( "Atlas" );
			}

			exit(0);
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

bool CApplication::HasCommand( const std::string& Command )
{
	auto Result = CommandLine.find( Command );
	return Result != CommandLine.end();
}

const std::string& CApplication::GetCommand( const std::string& Command )
{
	return CommandLine[Command];
}

void CApplication::SetFPSLimit( const int& Limit )
{
	FPSLimit = Limit;
}

bool CApplication::IsPaused()
{
	return PauseGame && !FrameStep;
}

#if defined(_WIN32)
#pragma comment(lib, "dbghelp.lib")

int GenerateDump( EXCEPTION_POINTERS* pExceptionPointers )
{
	auto DumpPathWide = CApplication::GetUserSettingsDirectory();
	const auto DirectoryName = CApplication::GetDirectoryName();

	if( DirectoryName.length() == 0 )
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	DumpPathWide += L"CrashDump.mdmp";
	const auto DumpPath = CApplication::UTF16ToUTF8( DumpPathWide );
	const HANDLE DumpFile = CreateFile( DumpPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0 );

	MINIDUMP_EXCEPTION_INFORMATION ExceptionInformation;
	ExceptionInformation.ThreadId = GetCurrentThreadId();
	ExceptionInformation.ExceptionPointers = pExceptionPointers;
	ExceptionInformation.ClientPointers = TRUE;

	MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), DumpFile, MiniDumpWithDataSegs, &ExceptionInformation, NULL, NULL );

	return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI ExceptionHandler( EXCEPTION_POINTERS* ExceptionInfo )
{
	GenerateDump( ExceptionInfo );

	const std::vector<Log::FHistory>& LogHistory = Log::History();
	if( LogHistory.size() > 0 )
	{
		Log::Event( Log::Fatal, "An exception has occured. Generating CrashDump.mdmp...\n\nLast known log entry:\n%s\n", LogHistory[LogHistory.size() - 1].Message.c_str() );
	}
	else
	{
		Log::Event( Log::Fatal, "An exception has occured. Generating CrashDump.mdmp...\n" );
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
	ImGui_ImplGlfw_Shutdown();
#endif

	const int EndianValue{ 0x01 };
	const auto* Address = static_cast<const void*>( &EndianValue );
	const auto* LeastSignificantAddress = static_cast<const unsigned char*>( Address );
	const bool LittleEndian = ( *LeastSignificantAddress == 0x01 );
	if( LittleEndian )
	{
		Log::Event( "Byte order: Little endian.\n" );
	}
	else
	{
		Log::Event( "Byte order: Big endian.\n" );
	}

	ServiceRegistry.CreateStandardServices();

	// Calling Get creates the instance and initializes the class.
	CConfiguration& Configuration = CConfiguration::Get();

	static const std::wstring UserConfigurationDirectory = GetUserSettingsDirectory();
	static const std::wstring UserConfigurationPath = GetUserSettingsPath();

	Log::Event( "User configuration path: %s\n", UTF16ToUTF8( UserConfigurationPath ).c_str() );
	Configuration.SetFile( StorageCategory::User, UserConfigurationPath );

	const bool DirectoryExists = std::experimental::filesystem::exists( UserConfigurationDirectory );
	const bool DirectoryValid = std::experimental::filesystem::is_directory( UserConfigurationDirectory );
	if( !DirectoryExists || !DirectoryValid )
	{
		std::experimental::filesystem::create_directory( UserConfigurationDirectory );
	}

	Configuration.Initialize();

	const bool ConfigurationPathExists = std::experimental::filesystem::exists( UserConfigurationPath );
	if( !ConfigurationPathExists )
	{
		Log::Event( "Seeding user configuration file. (file doesn't exist, first save)\n" );
		Configuration.Save();
	}
	else
	{
		Log::Event( "Found existing user configuration file.\n" );
	}

	if( MainWindow.Valid() )
	{
		MainWindow.Terminate();
		ImGui_ImplGlfw_Reset();
		ImGui_ImplOpenGL3_Reset();
	}

	MainWindow.Create( Name.c_str() );

	if( !MainWindow.Valid() && !MainWindow.IsWindowless() )
	{
		Log::Event( Log::Fatal, "Application window could not be created.\n" );
	}

	ResetImGui();

	// Render a single frame to indicate we're initializing.
	if( !MainWindow.ShouldClose() && !MainWindow.IsWindowless() )
	{
		MainWindow.BeginFrame();

#if defined( IMGUI_ENABLED )
		ImGui::SetNextWindowPos( ImVec2( 0.0f, 0.0f ), ImGuiCond_Always );
		ImGui::SetNextWindowSize( ImVec2( 500.0f, 20.0f ), ImGuiCond_Always );

		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) ); // Transparent background
		ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) ); // Transparent border
		if( ImGui::Begin( "Loading", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
		{
			ImGui::Text( "..." );
			ImGui::End();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
#endif

		MainWindow.RenderFrame();
	}

#if defined( IMGUI_ENABLED )
	if( !MainWindow.IsWindowless() )
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
	}
#endif

	ResetImGui();

	Log::Event( "Binding engine inputs.\n" );

	GLFWwindow* WindowHandle = MainWindow.Handle();
	if( !MainWindow.IsWindowless() )
	{
		glfwSetKeyCallback( WindowHandle, InputKeyCallback );
		glfwSetCharCallback( WindowHandle, InputCharCallback );
		glfwSetMouseButtonCallback( WindowHandle, InputMouseButtonCallback );
		glfwSetCursorPosCallback( WindowHandle, InputMousePositionCallback );
		glfwSetScrollCallback( WindowHandle, InputScrollCallback );
		glfwSetJoystickCallback( InputJoystickStatusCallback );

		for( int Index = 0; Index < GLFW_JOYSTICK_LAST; Index++ )
		{
			if( glfwJoystickPresent( Index ) )
			{
				InputJoystickStatusCallback( Index, GLFW_CONNECTED );
			}
		}
	}

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

	if( WaitForInput && !MainWindow.IsWindowless() )
	{
		while( glfwGetKey( WindowHandle, 32 ) != GLFW_PRESS )
		{
			glfwPollEvents();
		}

		WaitForInput = false;
	}

	CSoLoudSound::Initialize();
	GameLayersInstance->Initialize();
}
