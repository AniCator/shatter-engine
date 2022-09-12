// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
#include <map>
#include <vector>
#include <Engine/Utility/Service/ServiceRegistry.h>
#include <Engine/Utility/Timer.h>

typedef std::function<void(bool)> DebugUIFunction;

struct AdditionalTick
{
	using TickFunction = void( * )( void );
	enum Execute
	{
		Frame = 0, // Executed once per frame.

		PreTick, // Executed before any accumulated ticks are run.
		Tick, // The main tick function that is called whenever ticks accumulate over time.
		PostTick, // Executed after all accumulated ticks have run.

		Maximum
	};

	void Register( const Execute& Location, TickFunction Function );
	void Unregister( const Execute& Location, TickFunction Function );

	std::vector<TickFunction> Functions[Execute::Maximum];
};

class CApplication
{
public:
	CApplication();
	~CApplication();

	void Initialize();
	void Run();
	void Close();

	void InitializeDefaultInputs();
	static void ResetImGui();

	static const std::string& GetName();
	static void SetName( const char* Name );

	// Returns the local directory name used for storing a user's settings.
	static const std::wstring& GetDirectoryName();

	// Sets the local directory name used for storing a user's settings.
	static void SetDirectoryName(const wchar_t* Name);

	static const std::wstring& GetUserSettingsDirectory();
	static const std::wstring& GetUserSettingsFileName();
	static const std::wstring& GetUserSettingsPath();
	static const std::wstring& GetApplicationDirectory();
	
	static std::string UTF16ToUTF8( const std::wstring& UTF16 );
	static std::wstring UTF8ToUTF16( const std::string& UTF8 );
	static std::wstring Relative( const std::wstring& UTF16 );
	static std::string Relative( const std::string& UTF8 );

	void RedirectLogToConsole();
	void ProcessCommandLine( int argc, char** argv );

	static bool ToolsEnabled();
	static void EnableTools( const bool& Enable );

	bool DefaultExitEnabled() const;
	void EnableDefaultExit( const bool Enable );

	void RegisterDebugUI( DebugUIFunction Function );
	void RenderDebugUI( const bool Menu );
	void UnregisterDebugUI();
	size_t DebugFunctions() const;

	bool HasCommand( const std::string& Command );
	const std::string& GetCommand( const std::string& Command );

	void SetFPSLimit( const int& Limit = 0 );

	static bool IsPaused();
	static void SetPause( const bool& Pause );

	static bool IsPowerSaving();
	static void SetPowerSaving( const bool& Enable );

	static bool IsSleeping();

	CServiceRegistry ServiceRegistry;

	AdditionalTick Ticks;

private:
	static const std::wstring& GetAppDataDirectory();

	static std::string Name;
	static std::wstring DirectoryName;
	static bool Tools;
	bool DefaultExit;
	bool WaitForInput;

	std::vector<DebugUIFunction> DebugUIFunctions;
	std::map<std::string, std::string> CommandLine;

	// Update related variables.
	int FPSLimit = 0;
	double MaximumGameTime = 1 / 60.0;
	double GameAccumulator = 0.0;

	Timer InputTimer = { false };
	Timer GameTimer = { true };
	Timer RenderTimer = { false };
	Timer RealTime = { false };

	// The main update method that executes game ticks and renders frames.
	void Update();
	bool UpdateFrame();
};
