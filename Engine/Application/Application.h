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
	static const std::string& GetDirectoryName();

	// Sets the local directory name used for storing a user's settings.
	static void SetDirectoryName(const char* Name);

	static std::string GetUserSettingsDirectory();

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

	CServiceRegistry ServiceRegistry;

private:
	static std::string Name;
	static std::string DirectoryName;
	bool Tools;
	bool DefaultExit;
	bool WaitForInput;

	std::vector<DebugUIFunction> DebugUIFunctions;
	std::map<std::string, std::string> CommandLine;
};
