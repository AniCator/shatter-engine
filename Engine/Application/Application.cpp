// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Application.h"
#include "ApplicationMenu.h"
#include "EngineAssetLoaders.h"
#include "Themes.h"

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
#include <stack>

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Configuration/Configuration.h>

#include <Engine/Profiling/Profiling.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Sequencer.h> // TODO: Move sequencer updating somewhere else (see Frame call below)
#include <Engine/Utility/Locator/InputLocator.h>
#include <Engine/Utility/Data.h>
#include <Engine/Utility/File.h>
#include <Engine/Utility/MeshBuilder.h>
#include <Engine/Utility/Script/AngelEngine.h>
#include <Engine/Utility/Thread.h>
#include <Engine/Utility/ThreadPool.h>
#include <Engine/World/World.h>

#include <Game/Game.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.70/imgui.h>
#include <Engine/Display/imgui_impl_glfw.h>
#include <Engine/Display/imgui_impl_opengl3.h>
#endif

#pragma warning(disable : 4996)

CWindow& MainWindow = CWindow::Get();

std::string CApplication::Name = "Unnamed Shatter Engine Application";
std::wstring CApplication::DirectoryName = L"UnnamedShatterGame";
bool CApplication::Tools = false;

CCamera DefaultCamera = CCamera();
FCameraSetup& Setup = DefaultCamera.GetCameraSetup();

// Determines if the game should be paused or not.
static bool PauseGame = false;

// Used to request the game to step forward by one tick. (mainly useful while the game is paused)
bool FrameStep = false;

// Lowers the time scale to 1/10th of the current value.
bool ScaleTime = false;

// Simulates an inconsistent framerate.
bool SimulateJitter = false;

// Restarts all game layers.
bool RestartLayers = false;

// Mode that reduces framerate based on input frequency.
bool PowerSaving = false;
double LastInputTime = -1.0;
FFixedPosition2D LastMousePosition;

std::atomic<bool> Sleeping( false );

ConfigurationVariable<bool> UseAccumulator( "tick.UseAccumulator", true );

#ifdef OptickBuild
bool CaptureFrame = false;
size_t Capturing = 0;
#endif

double ScaledGameTime = 0.0;

constexpr float SlowMotionTarget = 0.1f;
void InputScaleTimeEnable( const float& Scale = 1.0f )
{
	ScaleTime = true;
	GameLayersInstance->SetTimeScale( Math::Lerp( GameLayersInstance->GetTimeScale(), SlowMotionTarget, Math::Saturate( GameLayersInstance->GetFrameTime() * 10.0f ) ) );
	SoLoudSound::Rate( Bus::SFX, Math::Max(0.01f, GameLayersInstance->GetTimeScale() ) );
}

void InputScaleTimeDisable( const float& Scale = 1.0f )
{
	ScaleTime = false;
	GameLayersInstance->SetTimeScale( 1.0f );
	SoLoudSound::Rate( Bus::SFX, 1.0f );
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

	ScriptEngine::Shutdown();
	ScriptEngine::Initialize();

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

std::vector<std::string> AutoCompleteCommand( const std::string& Partial )
{
	if( Partial.empty() )
		return {};

	std::string Query = Partial;
	std::string Comparison;

	// Transform to lowercase.
	std::transform( Query.begin(), Query.end(), Query.begin(), ::tolower );

	std::vector<std::string> Result;
	auto& Configuration = CConfiguration::Get();
	const auto& Settings = Configuration.GetSettings();
	for( const auto& Pair : Settings )
	{
		// Transform to lowercase.
		Comparison = Pair.first;
		std::transform( Comparison.begin(), Comparison.end(), Comparison.begin(), ::tolower );

		// Check if the lowercase characters are present.
		if( Comparison.find( Query ) != std::string::npos )
		{
			Result.emplace_back( Pair.first );
		}
	}

	const auto& Commands = Configuration.GetCommands();
	for( const auto& Pair : Commands )
	{
		// Transform to lowercase.
		Comparison = Pair.first;
		std::transform( Comparison.begin(), Comparison.end(), Comparison.begin(), ::tolower );

		// Check if the lowercase characters are present.
		if( Comparison.find( Query ) != std::string::npos )
		{
			Result.emplace_back( Pair.first );
		}
	}

	std::sort( Result.begin(), Result.end(), []( const std::string& A, const std::string& B ) {
		return A.length() > B.length();
	} );

	return Result;
}

bool ExecuteConsoleCommand( const std::string& Command )
{
	Log::Event( "%s\n", Command.c_str() );
	const auto Pair = String::Split( Command, ' ' );
	if( Pair.first.empty() )
		return false; // Invalid data.

	auto& Configuration = CConfiguration::Get();

	if( !Configuration.IsValidKey( Pair.first ) )
	{
		// Check if it's a command instead.
		if( Configuration.IsValidCommand( Pair.first ) )
		{
			// Execute the command.
			Configuration.Execute( Pair.first, Pair.second );
			return true;
		}

		Log::Event( "%s is not a valid command.\n", Pair.first );
		return false;
	}

	if( Pair.second.empty() )
	{
		if( !Configuration.IsValidCommand( Pair.first ) )
		{
			Log::Event( "%s = %s\n", Pair.first.c_str(), Configuration.GetValue( Pair.first ).c_str() );
		}

		return false;
	}

	Configuration.Store( Pair.first, Pair.second );
	return true;
}

const std::string ColorNames[ImGuiCol_::ImGuiCol_COUNT] = {
	"Text",
	"TextDisabled",
	"WindowBg",
	"ChildBg",
	"PopupBg",
	"Border",
	"BorderShadow",
	"FrameBg",
	"FrameBgHovered",
	"FrameBgActive",
	"TitleBg",
	"TitleBgActive",
	"TitleBgCollapsed",
	"MenuBarBg",
	"ScrollbarBg",
	"ScrollbarGrab",
	"ScrollbarGrabHovered",
	"ScrollbarGrabActive",
	"CheckMark",
	"SliderGrab",
	"SliderGrabActive",
	"Button",
	"ButtonHovered",
	"ButtonActive",
	"Header",
	"HeaderHovered",
	"HeaderActive",
	"Separator",
	"SeparatorHovered",
	"SeparatorActive",
	"ResizeGrip",
	"ResizeGripHovered",
	"ResizeGripActive",
	"Tab",
	"TabHovered",
	"TabActive",
	"TabUnfocused",
	"TabUnfocusedActive",
	"PlotLines",
	"PlotLinesHovered",
	"PlotHistogram",
	"PlotHistogramHovered",
	"TextSelectedBg",
	"DragDropTarget",
	"NavHighlight",
	"NavWindowingHighlight",
	"NavWindowingDimBg",
	"ModalWindowDimBg",
};

std::map<std::string, std::function<void()>> Themes;
void GenerateThemes()
{
	Themes.insert_or_assign( "ImGui", ThemeDefault );
	ThemeDefault();

	Themes.insert_or_assign( "Cherry", ThemeCherry );
	Themes.insert_or_assign( "Dracula", ThemeDracula );
	Themes.insert_or_assign( "Gruvbox (Dark)", ThemeGruvboxDark );
	Themes.insert_or_assign( "Gruvbox (Light)", ThemeGruvboxLight );

	const auto& Theme = CConfiguration::Get().GetString( "UI.Tool.Theme", "Gruvbox (Dark)" );
	if ( Theme == "Gruvbox (Light)" )
	{
		ThemeGruvboxLight();
	}
	else if( Theme == "Cherry" )
	{
		ThemeCherry();
	}
	else if( Theme == "Dracula" )
	{
		ThemeDracula();
	}
	else if( Theme == "ImGui" )
	{
		ThemeDefault();
	}
	else
	{
		// Default to the dark theme.
		ThemeGruvboxDark();
	}

	// ThemeGruvboxDark();
	// const auto& Style = ImGui::GetStyle();
	// for( size_t Index = 0; Index < ImGuiCol_::ImGuiCol_COUNT; ++Index )
	// {
	// 	Log::Event( "#%08x (%s)\n", ImGui::GetColorU32( Style.Colors[Index] ), ColorNames[Index].c_str() );
	// }
}

void SetTheme( const std::string& Theme )
{
	const auto& Iterator = Themes.find( Theme );
	if( Iterator != Themes.end() )
	{
		Iterator->second();

		CConfiguration::Get().Store( "UI.Tool.Theme", Theme );
		CConfiguration::Get().Save();
	}
}

static bool ShowTestWindow = false;
static bool ShowMetricsWindow = false;

static bool DisplayLog = false;
static size_t PreviousSize = 0;
void DebugMenu( CApplication* Application )
{
	if( !Application || !GameLayersInstance )
		return;

	if( CApplication::ToolsEnabled() && ImGui::BeginMainMenuBar() )
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

		const auto MenuName = CApplication::GetName() + " " + Number.String();
		if( ImGui::BeginMenu( MenuName.c_str() ) )
		{
			const auto BuildNumber = "Build " + std::to_string( Number.Hot );
			ImGui::MenuItem( BuildNumber.c_str(), nullptr, false, false );
			ImGui::MenuItem( "2017 \xc2\xa9 Christiaan Bakker", nullptr, false, false );

			if( ImGui::MenuItem( "Quit" ) )
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
				if( ScaleTime )
				{
					GameLayersInstance->SetTimeScale( SlowMotionTarget );
				}
				else
				{
					GameLayersInstance->SetTimeScale( 1.0f );
				}
			}

			if( ImGui::MenuItem( "Simulate Frame Jitter", nullptr, SimulateJitter ) )
			{
				SimulateJitter = !SimulateJitter;
			}

			if( ImGui::MenuItem( "Power Save Mode", nullptr, PowerSaving ) )
			{
				PowerSaving = !PowerSaving;

				if( !PowerSaving )
				{
					// Default to configured amount when disabling power saving.
					Application->SetFPSLimit( CConfiguration::Get().GetInteger( "render.FPS", 0 ) );
				}
			}

			// Render any remaining items from ApplicationMenu.cpp.
			RenderCommandItems();

			ImGui::Separator();

			if( ImGui::MenuItem( "800x600" ) )
			{
				MainWindow.Resize( ViewDimensions( 800, 600 ) );
			}

			if( ImGui::MenuItem( "1280x720" ) )
			{
				MainWindow.Resize( ViewDimensions( 1280, 720 ) );
			}

			if( ImGui::MenuItem( "1920x1080" ) )
			{
				MainWindow.Resize( ViewDimensions( 1920, 1080 ) );
			}

			if( ImGui::MenuItem( "2560x1440" ) )
			{
				MainWindow.Resize( ViewDimensions( 2560, 1440 ) );
			}

			if( ImGui::MenuItem( "Reload Configuration" ) )
			{
				InputReloadConfiguration();
			}

			if( ImGui::MenuItem( "Restart Game Layers" ) )
			{
				RestartLayers = true;
			}

			if( ImGui::MenuItem( "Reload Shaders" ) )
			{
				InputReloadShaders();
			}

			const bool Enabled = MainWindow.GetRenderer().ForceWireFrame;
			if( ImGui::MenuItem( "Toggle Wireframe", nullptr, Enabled ) )
			{
				MainWindow.GetRenderer().ForceWireFrame = !Enabled;
			}

#ifdef OptickBuild
			if( ImGui::MenuItem( "Profile 20 Frames (Optick)" ) )
			{
				CaptureFrame = true;
			}
#endif

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

			ImGui::Separator();

			RenderWindowItems();

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

		auto LogBackground = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		LogBackground.w = 0.65f;

		ImGui::PushStyleColor( ImGuiCol_WindowBg, LogBackground ); // Transparent background

		if( ImGui::Begin( "Log", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar ) )
		{
			static const char* SeverityToString[Log::LogMax] =
			{
				"",
				"Warning",
				"Error",
				"Fatal",
				"Assert"
			};

			static const ImVec4 SeverityToColor[Log::LogMax] =
			{
				ImVec4( 1.0f,1.0f,1.0f,1.0f ),
				ImVec4( 1.0f,1.0f,0.0f,1.0f ),
				ImVec4( 1.0f,0.0f,0.0f,1.0f ),
				ImVec4( 1.0f,0.0f,0.0f,1.0f ),
				ImVec4( 0.1f,0.4f,1.0f,1.0f ),
			};

			const auto Region = ImGui::GetContentRegionAvail();
			ImGui::BeginChild( "LogText", { Region.x, Region.y - 20.0f }, false, ImGuiWindowFlags_NoSavedSettings );

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
				ImGui::SetScrollHereY( 1.0f );
				PreviousSize = Count;
			}

			ImGui::EndChild();
			ImGui::BeginChild( "Console", { Region.x, 20.0f }, false, ImGuiWindowFlags_NoSavedSettings );

			constexpr int GraveAccent = 96;
			const bool ConsoleKey = ImGui::IsKeyDown( GLFW_KEY_GRAVE_ACCENT );
			if( !ConsoleKey )
			{
				ImGui::SetNextItemWidth( -1.0f );
				ImGui::SetItemDefaultFocus();
				ImGui::SetKeyboardFocusHere();

				static std::string PreviousCommand;
				static std::array<char, 2048> ConsoleInput;
				ImGuiInputTextFlags Flags = 0;

				static int AutoIndex = -1;
				auto AutoComplete = AutoCompleteCommand( ConsoleInput.data() );
				if( std::char_traits<char>::length( ConsoleInput.data() ) < 2 )
				{
					AutoComplete.clear();
				}

				if( AutoIndex < 0 )
				{
					AutoIndex = AutoComplete.size() - 1;
				}

				if( AutoIndex >= AutoComplete.size() )
				{
					AutoIndex = 0;
				}

				bool SimulatedInput = false;
				if( ImGui::IsKeyDown( GLFW_KEY_TAB ) && !AutoComplete.empty() )
				{
					std::string Choice = AutoComplete[AutoIndex];
					Choice += " ";
					Choice.copy( ConsoleInput.data(), ConsoleInput.size() );
					Flags |= ImGuiInputTextFlags_ReadOnly;

					// Simulate an end-key press event.
					ImGui::GetIO().KeysDownDuration[ImGui::GetKeyIndex( ImGuiKey_End )] = 0.0f;
					SimulatedInput = true;
				}

				if( ImGui::IsKeyPressed( GLFW_KEY_UP ) )
				{
					if( AutoComplete.empty() )
					{
						PreviousCommand.copy( ConsoleInput.data(), ConsoleInput.size() );

						// Simulate an end-key press event.
						ImGui::GetIO().KeysDownDuration[ImGui::GetKeyIndex( ImGuiKey_End )] = 0.0f;
						SimulatedInput = true;
					}
					else
					{
						AutoIndex--;
					}

					Flags |= ImGuiInputTextFlags_ReadOnly;
				}

				if( ImGui::IsKeyPressed( GLFW_KEY_DOWN ) )
				{
					if( AutoComplete.empty() )
					{
						PreviousCommand.copy( ConsoleInput.data(), ConsoleInput.size() );

						// Simulate an end-key press event.
						ImGui::GetIO().KeysDownDuration[ImGui::GetKeyIndex( ImGuiKey_End )] = 0.0f;
						SimulatedInput = true;
					}
					else
					{
						AutoIndex++;
					}

					Flags |= ImGuiInputTextFlags_ReadOnly;
				}

				if( AutoIndex < 0 )
				{
					AutoIndex = AutoComplete.size() - 1;
				}

				if( AutoIndex >= AutoComplete.size() )
				{
					AutoIndex = 0;
				}

				if( ImGui::InputText( "##ConsoleInput", ConsoleInput.data(), ConsoleInput.size(), Flags ) )
				{
					AutoComplete = AutoCompleteCommand( ConsoleInput.data() );
					AutoIndex = AutoComplete.size() - 1;
				}

				if( ImGui::IsKeyPressed( GLFW_KEY_ENTER ) || ImGui::IsKeyPressed( GLFW_KEY_KP_ENTER ) )
				{
					PreviousCommand = ConsoleInput.data();

					// Clear the input field.
					ConsoleInput.fill( '\0' );

					if( ExecuteConsoleCommand( PreviousCommand ) )
					{
						// Close the logger once a command is executed.
						DisplayLog = false;
					}
				}

				ImGui::ResetNavigationToItem();

				if( SimulatedInput )
				{
					// Reset the key state.
					const auto Key = ImGui::GetKeyIndex( ImGuiKey_End );
					ImGui::GetIO().KeysDownDuration[Key] = ImGui::GetIO().KeysDownDurationPrev[Key];
				}

				if( !AutoComplete.empty() && !AutoComplete.back().empty() )
				{
					auto Position = ImGui::GetCursorScreenPos();
					Position.y -= 45.0f + ( AutoComplete.size() - 1) * 18.0f;
					ImGui::SetNextWindowPos( Position );
					ImGui::BeginTooltip();

					int Index = 0;
					for( const auto& String : AutoComplete )
					{
						if( Index == AutoIndex )
						{
							ImGui::TextColored( ImVec4( 0.33f, 0.75f, 1.0f, 1.0f ), "%s", String.c_str() );
						}
						else
						{
							ImGui::Text( "%s", String.c_str() );
						}

						Index++;
					}

					ImGui::EndTooltip();
				}
			}

			ImGui::EndChild();
		}
		ImGui::End();

		ImGui::PopStyleColor();
	}

	RenderWindowPanels();

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

void RenderToolUI( CApplication* Application )
{
#if defined( IMGUI_ENABLED )
	if( !MainWindow.IsWindowless() )
	{
		CProfiler::Get().Display();
		CProfiler::Get().ClearFrame();
		DebugMenu( Application );
	}
#endif
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

void PollInput()
{
	// Always poll input.
	if( !MainWindow.IsWindowless() )
	{
		SetMouseWheel( ImGui::GetIO().MouseWheel );
	}

	MainWindow.ProcessInput();
}

void CApplication::Run()
{
	Initialize();

	Setup.AspectRatio = static_cast<float>( MainWindow.GetWidth() ) / static_cast<float>( MainWindow.GetHeight() );
	auto& Renderer = MainWindow.GetRenderer();
	Renderer.SetCamera( DefaultCamera );

	InputTimer.Start();
	GameTimer.Start();
	RenderTimer.Start();
	RealTime.Start();

	SetFPSLimit( CConfiguration::Get().GetInteger( "render.FPS", 0 ) );

	MaximumGameTime = 1.0 / CConfiguration::Get().GetInteger( "tick.Rate", 60 );

	const float GlobalVolume = CConfiguration::Get().GetFloat( "audio.MasterVolume", 100.0f );
	SoLoudSound::Volume( GlobalVolume );

	while( !MainWindow.ShouldClose() )
	{
		ProfileFrame( "MainThread" );
		Update();
	}

	ScriptEngine::Shutdown();

	GameLayersInstance->Shutdown();
	delete GameLayersInstance;

	MainWindow.Terminate();
	SoLoudSound::Shutdown();
	ThreadPool::Shutdown();
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

	Input.AddActionBinding( "TimeScaleEnable", EMouse::MouseButton5, EAction::Press, InputScaleTimeEnable );
	Input.AddActionBinding( "TimeScaleDisable", EMouse::MouseButton5, EAction::Release, InputScaleTimeDisable );

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

	Input.AddActionBinding( "DisplayLog", EKey::GraveAccent, EAction::Release, [] ( const float& Scale ) {
		DisplayLog = !DisplayLog;
	} );

	Input.AddActionBinding( "FrameStep", EKey::F5, EAction::Release, [] ( const float& Scale ) {
		FrameStep = true;
		} );

	Input.AddActionBinding( "Pause", EKey::F6, EAction::Release, [] ( const float& Scale ) {
		PauseGame = !PauseGame;
		} );

	Input.AddActionBinding( "ShowToolbar", EKey::NumpadSubtract, EAction::Release, [&] ( const float& Scale ) {
		EnableTools( !Tools );
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
	IO.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	IO.ConfigFlags &= ~ImGuiConfigFlags_NavEnableSetMousePos;

	static const char* FontLocation = "Resources/Roboto-Medium.ttf";
	const bool FileExists = CFile::Exists( FontLocation );
	if( FileExists )
	{
		ImGuiIO& IO = ImGui::GetIO();
		static ImFont* RobotoFont = nullptr;

		ImFontConfig DefaultFontConfig;
		DefaultFontConfig.OversampleH = 4;
		DefaultFontConfig.OversampleV = 2;
		DefaultFontConfig.SizePixels = CConfiguration::Get().GetFloat( "UI.FontSize", 15.0f );

		static ImVector<ImWchar> GlyphRange;
		ImFontGlyphRangesBuilder Builder;
		Builder.AddRanges( IO.Fonts->GetGlyphRangesDefault() );
		Builder.BuildRanges( &GlyphRange );

		RobotoFont = IO.Fonts->AddFontFromFileTTF( FontLocation, DefaultFontConfig.SizePixels, &DefaultFontConfig, GlyphRange.Data );
		IO.FontDefault = RobotoFont;
	}
	else
	{
		Log::Event( Log::Warning, "Could not find a default resource.\nMake sure you include the default engine items in the build directory.\n" );
	}

	while( !ImGui::GetIO().Fonts->IsBuilt() )
	{
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

void AdditionalTick::Register( const Execute& Location, TickFunction Function )
{
	Functions[Location].emplace_back( Function );
}

void AdditionalTick::Unregister( const Execute& Location, TickFunction Function )
{
	// TODO: Fix this.
	return;
	auto Iterator = Functions[Location].begin();
	while( Iterator != Functions[Location].end() )
	{
		if( *Iterator == Function )
		{
			std::iter_swap( Iterator, Functions[Location].end() );
			Functions[Location].pop_back();
			return;
		}

		Iterator++;
	}
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

void ReleaseConsole()
{
	FILE* Reference;

	// Just to be safe, redirect standard IO to NUL before releasing.

	// Redirect STDIN to NUL
	if( freopen_s( &Reference, "NUL:", "r", stdin ) == 0 )
		setvbuf( stdin, NULL, _IONBF, 0 );

	// Redirect STDOUT to NUL
	if( freopen_s( &Reference, "NUL:", "w", stdout ) == 0 )
		setvbuf( stdout, NULL, _IONBF, 0 );

	if( freopen_s( &Reference, "NUL:", "w", stderr ) == 0 )
		setvbuf( stderr, NULL, _IONBF, 0 );

	// Detach from console
	FreeConsole();
}

void SetBufferSize( int16_t MinimumLength )
{
	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &conInfo );
	if( conInfo.dwSize.Y < MinimumLength )
		conInfo.dwSize.Y = MinimumLength;
	SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), conInfo.dwSize );
}

void CApplication::RedirectLogToConsole()
{
#if defined(_WIN32)
	ReleaseConsole();

	bool Attached = AttachConsole( ATTACH_PARENT_PROCESS );
	if( !Attached )
	{
		Attached = AllocConsole();
	}

	if( !Attached )
		return;

	Log::CLog::Get().ToStdOut = true;

	FILE* fp;
	if( freopen_s( &fp, "CONOUT$", "w", stdout ) == 0 )
		setvbuf( stdout, NULL, _IONBF, 0 );

	std::ios::sync_with_stdio();
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

		if( strcmp( argv[Index], "-profiler" ) == 0 )
		{
			CProfiler::Get().SetEnabled( true );
		}
		else if( strcmp( argv[Index], "-tools" ) == 0 )
		{
			EnableTools( true );
		}
		else if( strcmp( argv[Index], "-convert" ) == 0 )
		{
			RedirectLogToConsole();

			if( Index + 1 < argc )
			{			
				Timer LoadTimer;
				Timer ParseTimer;

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

					CFile File( ExportPath );
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

			if( !IsDebuggerPresent() )
			{
				RedirectLogToConsole();
			}
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

bool CApplication::ToolsEnabled()
{
	return Tools;
}

void CApplication::EnableTools( const bool& Enable )
{
	Tools = Enable;

	if( !Tools )
	{
		if( MainWindow.IsCursorEnabled() )
		{
			MainWindow.EnableCursor( false );
		}
	}
}

bool CApplication::DefaultExitEnabled() const
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

size_t CApplication::DebugFunctions() const
{
	return DebugUIFunctions.size();
}

bool CApplication::HasCommand( const std::string& Command )
{
	const auto Result = CommandLine.find( Command );
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

void CApplication::SetPause( const bool& Pause )
{
	PauseGame = Pause;
}

bool CApplication::IsPowerSaving()
{
	return PowerSaving;
}

void CApplication::SetPowerSaving( const bool& Enable )
{
	PowerSaving = Enable;
}

bool CApplication::IsSleeping()
{
	return Sleeping;
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

void SetupUserDirectories()
{
	const auto& UserConfigurationDirectory = CApplication::GetUserSettingsDirectory();
	const bool DirectoryExists = std::experimental::filesystem::exists( UserConfigurationDirectory );
	const bool DirectoryValid = std::experimental::filesystem::is_directory( UserConfigurationDirectory );
	if( !DirectoryExists || !DirectoryValid )
	{
		std::experimental::filesystem::create_directory( UserConfigurationDirectory );
	}
}

void CApplication::Initialize()
{
	ProfileScope();
#if defined(_WIN32)
	// Attach an exception handler.
	::SetUnhandledExceptionFilter( ExceptionHandler );
#endif

	// Configure the user directories first so that logs can be written to them.
	SetupUserDirectories();

	Log::Event( "%s (Build: %s)\n\n", Name.c_str(), __DATE__ );

#if defined( IMGUI_ENABLED )
	ImGui_ImplGlfw_Shutdown();
#endif

	constexpr int EndianValue{ 0x01 };
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

	if( !CommandLine.empty() )
	{
		Log::Event( "\nCommand line:\n" );
		for( auto& Line : CommandLine )
		{
			Log::Event( "%s | %s\n", Line.first.c_str(), Line.second.c_str() );
		}
	}

	ServiceRegistry.CreateStandardServices();

	// Initialize the symbol pool.
	NamePool::Get().Pool();

	// Configure the thread pool.
	ThreadPool::Initialize();

	// Calling Get creates the instance and initializes the class.
	CConfiguration& Configuration = CConfiguration::Get();

	const auto& UserConfigurationPath = GetUserSettingsPath();
	Log::Event( "User configuration path: %s\n", UTF16ToUTF8( UserConfigurationPath ).c_str() );
	Configuration.SetFile( StorageCategory::User, UserConfigurationPath );

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

	// TODO: Does this still have any use here?
	MainWindow.EnableCursor( true );

	if( !GameLayersInstance )
	{
		Log::Event( Log::Fatal, "Game layers instance does not exist!\n" );
	}

	if( CameraSpeed < 0.0f )
	{
		CameraSpeed = 0.1f;
	}

	UnregisterDebugUI();

	ScriptEngine::Initialize();

	// Initialize engine asset types.
	InitializeEngineAssetLoaders();

	if( WaitForInput && !MainWindow.IsWindowless() )
	{
		while( glfwGetKey( WindowHandle, 32 ) != GLFW_PRESS )
		{
			glfwPollEvents();
		}

		WaitForInput = false;

		MainWindow.EnableCursor( false );
	}

	SoLoudSound::Initialize();
	GameLayersInstance->Initialize();
}

void CApplication::Update()
{
#ifdef OptickBuild
	if( Capturing > 0 )
	{
		Capturing++;
	}

	if( CaptureFrame )
	{
		Capturing++;
		CaptureFrame = false;
		OptickStart();
	}
#endif

	CConfiguration::Get().ReloadIfModified();

	if( RestartLayers )
		InputRestartGameLayers( this );

	if( SimulateJitter )
	{
		Timer JitterTimer;
		JitterTimer.Start();
		const int64_t JitterTime = Math::RandomRangeInteger( 0, 33 );
		while( JitterTimer.GetElapsedTimeMilliseconds() < JitterTime )
		{
			// Wait a little bit to induce jitter.
		}
	}

	GameLayersInstance->RealTime( RealTime.GetElapsedTimeSeconds() );

	const double GameDeltaTime = GameTimer.GetElapsedTimeSeconds();
	GameAccumulator += GameDeltaTime;

	const auto Frozen = ( PauseGame && !FrameStep );
	const auto ExecuteTicks = GameAccumulator > MaximumGameTime;
	if( !Frozen && ExecuteTicks )
	{
		for( const auto& PreTick : Ticks.Functions[AdditionalTick::PreTick] )
		{
			PreTick();
		}
	}

	// Reset the accumulator to one tick if we're running particularily slow.
	if( GameAccumulator > ( MaximumGameTime * 4 ) )
	{
		GameAccumulator = MaximumGameTime;
	}

	int64_t TickCounter = 0; // Used for tick profiling.
	while( GameAccumulator >= MaximumGameTime )
	{
		TickCounter++;

		GameAccumulator -= MaximumGameTime;

		if( GameAccumulator < 0 )
		{
			// We're going to miss our ride, leave it for the next round.
			break;
		}

		if( Tools )
		{
			MainWindow.EnableCursor( true );
		}

		if( !Frozen )
		{
			if( !Tools )
			{
				MainWindow.EnableCursor( false );
			}

			CProfiler::Get().Clear();

			auto& Renderer = MainWindow.GetRenderer();
			Renderer.RefreshFrame();

			double PreviousTime = ScaledGameTime;
			if( UseAccumulator )
			{
				ScaledGameTime += MaximumGameTime;
			}
			else
			{
				ScaledGameTime += GameDeltaTime;
			}

			// Update game time.
			GameLayersInstance->Time( ScaledGameTime - PreviousTime );

			// Update the renderable stage for tick functions.
			Renderer.UpdateRenderableStage( RenderableStage::Tick );

			CInputLocator::Get().Tick();

			// Tick all game layers
			GameLayersInstance->Tick();

			for( const auto& Tick : Ticks.Functions[AdditionalTick::Tick] )
			{
				Tick();
			}

#ifdef OptickBuild
			if( Capturing >= 20 )
			{
				Capturing = 0;
				OptickStop();
				OptickSave( "OptickCapture.opt" );
			}
#endif

			if( FrameStep )
			{
				FrameStep = false;
			}
		}

		// Only tick once if we don't want to use the accumulator.
		if( !UseAccumulator )
		{
			break;
		}
	}

	if( ExecuteTicks )
	{
		CProfiler& Profiler = CProfiler::Get();
		const auto TickEntry = ProfileTimeEntry( "Ticks Performed", TickCounter );
		Profiler.AddCounterEntry( TickEntry, false, true );
	}

	if( !Frozen && ExecuteTicks )
	{
		for( const auto& PostTick : Ticks.Functions[AdditionalTick::PostTick] )
		{
			PostTick();
		}

		GameTimer.Start();
	}

	if( Frozen )
	{
		GameTimer.Start();
		CInputLocator::Get().Tick();
	}

	PollInput();

	// BUG: FPS doesn't get restored after minimizing/power-saving.
	if( IsPowerSaving() ) // || MainWindow.IsMinimized() )
	{
		const auto& Input = CInputLocator::Get();
		const auto MousePosition = Input.GetMousePosition();
		auto DeltaPosition = LastMousePosition;
		DeltaPosition.X -= MousePosition.X;
		DeltaPosition.Y -= MousePosition.Y;

		DeltaPosition.X = abs( DeltaPosition.X );
		DeltaPosition.Y = abs( DeltaPosition.Y );

		const auto MouseMoved = DeltaPosition.X > 0 || DeltaPosition.Y > 0;
		const auto CurrentTime = RealTime.GetElapsedTimeSeconds();

		if( Input.IsAnyKeyDown() || MouseMoved || LastInputTime < 0.0 )
		{
			LastInputTime = CurrentTime;
			LastMousePosition = MousePosition;

			FPSLimit = 0;
		}

		int FPSTarget = 0;
		const double TimeSinceInput = CurrentTime - LastInputTime;
		if( TimeSinceInput > 30.0 )
		{
			FPSTarget = 5;
		}
		else if( TimeSinceInput > 20.0 )
		{
			FPSTarget = 15;
		}
		else if( TimeSinceInput > 10.0 )
		{
			FPSTarget = 30;
		}
		else if( TimeSinceInput > 4.0 )
		{
			FPSTarget = 60;
		}

		if( TimeSinceInput > 4.0 && FPSLimit < 1 )
		{
			FPSLimit = 300;
		}

		FPSLimit = FPSLimit * 0.99999 + FPSTarget * 0.00001;

		const auto ConfiguredFPS = CConfiguration::Get().GetInteger( "render.FPS", 0 );
		if( ConfiguredFPS > 0 )
		{
			FPSLimit = Math::Min( FPSLimit, ConfiguredFPS );
		}

		if( ( !MainWindow.IsFocused() && TimeSinceInput > 1.0 ) || TimeSinceInput > 6.0 )
		{
			Sleeping = true;
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}
		else
		{
			Sleeping = false;
		}
	}
	else
	{
		Sleeping = false;
	}

	if( UpdateFrame() )
	{
		// Update the cursor after ticking the game layers so that state changes aren't lost.
		MainWindow.UpdateCursor();
	}
}

bool CApplication::UpdateFrame()
{
	const auto UnboundedFramerate = FPSLimit < 1;
	const double RenderDeltaTime = RenderTimer.GetElapsedTimeSeconds();
	const double MaximumFrameTime = UnboundedFramerate ? RenderDeltaTime : 1.0 / FPSLimit;
	const auto ShouldRender = !MainWindow.IsMinimized() && ( RenderDeltaTime > MaximumFrameTime || UnboundedFramerate );
	if( !ShouldRender )
		return false;

	TimerScope::Submit( "Frametime", RenderTimer.GetStartTime(), RenderTimer.GetElapsedTimeMilliseconds() );
	GameLayersInstance->FrameTime( RenderDeltaTime );
	RenderTimer.Start( 0 );

	// Prepare to render a new frame.
	MainWindow.BeginFrame();
	MainWindow.GetRenderer().UpdateRenderableStage( RenderableStage::Frame );

	// Gather everything for rendering.
	GameLayersInstance->Frame();

	// TODO: Sequence update should happen elsewhere but this is better than having it run via DebugMenu().
	for( auto& Sequence : CAssets::Get().Sequences.GetAssets() )
	{
		Sequence->Frame();
	}

	// Prepare the UI of the profiler and other debug menus.
	RenderToolUI( this );

	// It's rendering time.
	MainWindow.RenderFrame();

	// Execute all commands and swap the front and back buffer.
	MainWindow.SwapFrame();

	if( !UnboundedFramerate )
	{
		// We're using the frame limiter, time for additional power saving.
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	return true;
}
