// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LoftyLagoon.h"

#include <Engine/Profiling/Profiling.h>
#include <Engine/Profiling/Logging.h>

#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Configuration/Configuration.h>

#include <Engine/Utility/Math.h>
#include <Engine/Utility/Math/SIMDVector.h>

#include <cstdlib>

#include <Engine/Application/Application.h>

#if defined( ConsoleWindowDisabled )
#if defined( _MSC_VER )
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

CGameLoftyLagoon* LoftyLagoonInstance = new CGameLoftyLagoon();

void main()
{
	if( !GameLayersInstance )
	{
		Log::Event( Log::Fatal, "Game layers instance does not exist!\n" );
	}

	GameLayersInstance->Add( LoftyLagoonInstance );

	CApplication Application;
	Application.Run();
}

CGameLoftyLagoon::CGameLoftyLagoon()
{
	
}

CGameLoftyLagoon::~CGameLoftyLagoon()
{

}

void CGameLoftyLagoon::Initialize()
{
	
}

void CGameLoftyLagoon::Frame()
{
	
}

void CGameLoftyLagoon::Tick()
{
	
}

void CGameLoftyLagoon::Shutdown()
{
	
}
