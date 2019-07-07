// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>

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

	std::string GetName() const;
	void SetName( const char* Name );

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

	CServiceRegistry ServiceRegistry;

private:
	std::string Name;
	bool Tools;
	bool DefaultExit;
	bool WaitForInput;

	std::vector<DebugUIFunction> DebugUIFunctions;
};
