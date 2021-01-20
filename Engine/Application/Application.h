// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
#include <map>
#include <Engine/Utility/Service/ServiceRegistry.h>

typedef std::function<void(bool)> DebugUIFunction;

class CApplication
{
public:
	CApplication();
	~CApplication();

	void Initialize();
	void Run();
	void Close();

	void InitializeDefaultInputs();
	void ResetImGui();

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

	const bool ToolsEnabled() const;
	void EnableTools( const bool Enable );

	const bool DefaultExitEnabled() const;
	void EnableDefaultExit( const bool Enable );

	void RegisterDebugUI( DebugUIFunction Function );
	void RenderDebugUI( const bool Menu );
	void UnregisterDebugUI();
	const size_t DebugFunctions() const;

	bool HasCommand( const std::string& Command );
	const std::string& GetCommand( const std::string& Command );

	void SetFPSLimit( const int& Limit = 0 );

	CServiceRegistry ServiceRegistry;

private:
	static const std::wstring& GetAppDataDirectory();

	static std::string Name;
	static std::wstring DirectoryName;
	bool Tools;
	bool DefaultExit;
	bool WaitForInput;

	int FPSLimit = 0;

	std::vector<DebugUIFunction> DebugUIFunctions;
	std::map<std::string, std::string> CommandLine;
};
