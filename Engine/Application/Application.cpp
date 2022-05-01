// Copyright © 2017, Christiaan Bakker, All rights reserved.
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

#include <Engine/Profiling/Logging.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Audio/SoLoudSound.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/UserInterface.h>
#include <Engine/Configuration/Configuration.h>

#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Sequencer.h> // TODO: Move sequencer updating somewhere else (see Frame call below)
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
bool CApplication::Tools = false;

CCamera DefaultCamera = CCamera();
FCameraSetup& Setup = DefaultCamera.GetCameraSetup();
static bool PauseGame = false;
bool FrameStep = false;
bool ScaleTime = false;
bool SimulateJitter = false;
bool CursorVisible = true;
bool RestartLayers = false;

#ifdef OptickBuild
bool CaptureFrame = false;
bool Capturing = false;
#endif

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
void GenerateThemes()
{
	Themes.insert_or_assign( "ImGui", ThemeDefault );
	ThemeDefault();

	Themes.insert_or_assign( "Cherry", ThemeCherry );
	Themes.insert_or_assign( "Dracula", ThemeDracula );
	Themes.insert_or_assign( "Gruvbox (Dark)", ThemeGruvboxDark );
	Themes.insert_or_assign( "Gruvbox (Light)", ThemeGruvboxLight );

	// Default theme
	ThemeGruvboxDark();
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
			}

			if( ImGui::MenuItem( "Simulate Frame Jitter", nullptr, SimulateJitter ) )
			{
				SimulateJitter = !SimulateJitter;
			}

			ImGui::Separator();

			//if( ImGui::MenuItem( "Re-initialize Application", "" ) )
			//{
			//	Application->Initialize();

			//	// Has to return when executed because imgui will be reset.
			//	return;
			//}

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
			if( ImGui::MenuItem( "Capture Optick Frame" ) )
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

		auto LogBackground = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		LogBackground.w = 0.65f;

		ImGui::PushStyleColor( ImGuiCol_WindowBg, LogBackground ); // Transparent background

		if( ImGui::Begin( "Log", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings ) )
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
	CRenderer& Renderer = MainWindow.GetRenderer();
	Initialize();

	Setup.AspectRatio = static_cast<float>( MainWindow.GetWidth() ) / static_cast<float>( MainWindow.GetHeight() );
	Renderer.SetCamera( DefaultCamera );

	Timer InputTimer( false );
	Timer GameTimer( true );
	Timer RenderTimer( false );

	InputTimer.Start();
	GameTimer.Start();
	RenderTimer.Start();

	Timer RealTime( false );
	RealTime.Start();

	SetFPSLimit( CConfiguration::Get().GetInteger( "fps", 300 ) );

	const uint64_t MaximumGameTime = 1000 / CConfiguration::Get().GetInteger( "tickrate", 60 );
	const uint64_t MaximumInputTime = 1000 / CConfiguration::Get().GetInteger( "pollingrate", 120 );

	const float GlobalVolume = CConfiguration::Get().GetFloat( "volume", 100.0f );
	SoLoudSound::Volume( GlobalVolume );

	static uint64_t GameAccumulator = 0.0;
	constexpr uint64_t GameDeltaTick = 1000 / 60;

	// const auto TimeScaleParameter = CConfiguration::Get().GetDouble( "timescale", 1.0 );

	while( !MainWindow.ShouldClose() )
	{
		ProfileFrame( "Main Thread" );

#ifdef OptickBuild
		if( CaptureFrame )
		{
			Capturing = true;
			CaptureFrame = false;
			OptickStart();
		}
#endif

		if( RestartLayers )
			InputRestartGameLayers( this );

		if( SimulateJitter )
		{
			Timer JitterTimer;
			JitterTimer.Start();
			const int64_t JitterTime = Math::RandomRangeInteger( 1, 33 );
			while( JitterTimer.GetElapsedTimeMilliseconds() < JitterTime )
			{
				// Wait a little bit to induce jitter.
			}
		}

		GameLayersInstance->RealTime( StaticCast<double>( RealTime.GetElapsedTimeMilliseconds() ) * 0.001 );

		const uint64_t GameDeltaTime = GameTimer.GetElapsedTimeMilliseconds();
		GameAccumulator += GameDeltaTime;

		const auto Frozen = PauseGame && !FrameStep;
		const auto ExecuteTicks = GameAccumulator > MaximumGameTime;
		if( !Frozen && ExecuteTicks )
		{
			for( const auto& PreTick : Ticks.Functions[AdditionalTick::PreTick] )
			{
				PreTick();
			}
		}

		while( GameAccumulator > MaximumGameTime )
		{
			GameAccumulator -= MaximumGameTime;

			if( GameAccumulator > ( MaximumGameTime * 4 ) )
			{
				GameAccumulator = MaximumGameTime;
			}

			MainWindow.EnableCursor( false );
			
			if( Tools )
			{
				if( !MainWindow.IsCursorEnabled() )
				{
					MainWindow.EnableCursor( true );
				}
			}

			// const auto NotFrozen = !PauseGame || FrameStep;
			if( !Frozen )
			{
				CProfiler::Get().Clear();
				Renderer.RefreshFrame();

				const auto GameTimeScale = GameLayersInstance->GetTimeScale();
				const auto TimeScale = ScaleTime ? GameTimeScale * 0.1 : GameTimeScale;

				ScaledGameTime += static_cast<double>( MaximumGameTime ) * 0.001 * TimeScale;

				// Update game time.
				GameLayersInstance->Time( ScaledGameTime );

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
				if( Capturing )
				{
					Capturing = false;
					OptickSave("OptickCapture.opt");
				}
#endif

				if( FrameStep )
				{
					FrameStep = false;
				}
			}
		}

		if( !Frozen && ExecuteTicks )
		{
			for( const auto& PostTick : Ticks.Functions[AdditionalTick::PostTick] )
			{
				PostTick();
			}
		}

		if( Frozen )
		{
			GameTimer.Start();
			CInputLocator::Get().Tick();
		}

		PollInput();

		const auto UnboundedFramerate = FPSLimit < 1;
		const uint64_t RenderDeltaTime = RenderTimer.GetElapsedTimeMilliseconds();
		const uint64_t MaximumFrameTime = UnboundedFramerate ? RenderDeltaTime : 1000 / FPSLimit;
		if( RenderDeltaTime > MaximumFrameTime || UnboundedFramerate )
		{
			TimerScope::Submit( "Frametime", RenderTimer.GetStartTime(), RenderDeltaTime );
			GameLayersInstance->FrameTime( StaticCast<double>( RenderDeltaTime ) * 0.001 );
			RenderTimer.Start( UnboundedFramerate ? 0 : RenderDeltaTime - MaximumFrameTime );

			MainWindow.BeginFrame();

			{
				Renderer.UpdateRenderableStage( RenderableStage::Frame );
				GameLayersInstance->Frame();

				// TODO: Sequence update should happen elsewhere but this is better than having it run via DebugMenu().
				for( auto* Sequence : CAssets::Get().Sequences.GetAssets() )
				{
					Sequence->Frame();
				}

#if defined( IMGUI_ENABLED )
				if( !MainWindow.IsWindowless() )
				{
					CProfiler::Get().Display();
					CProfiler::Get().ClearFrame();
					DebugMenu( this );
				}
#endif

				MainWindow.RenderFrame();
			}
		}

		// Update the cursor after ticking the game layers so that state changes aren't lost.
		MainWindow.UpdateCursor();
	}

	// CAngelEngine::Get().Shutdown();

	GameLayersInstance->Shutdown();
	delete GameLayersInstance;

	MainWindow.Terminate();

	SoLoudSound::Shutdown();
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

	// Initialize the pool.
	NamePool::Get().Pool();

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
