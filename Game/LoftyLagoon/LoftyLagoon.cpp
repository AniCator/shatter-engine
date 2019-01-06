// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LoftyLagoon.h"

#include <Engine/Profiling/Logging.h>

#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Configuration/Configuration.h>

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
	Application.SetName( "Lofty Lagoon" );
	Application.Run();
}

std::unique_ptr<Log::CLog> CGameLoftyLagoon::Report( new Log::CLog( "LoftyLagoon" ) );

CGameLoftyLagoon::CGameLoftyLagoon()
{
	
}

CGameLoftyLagoon::~CGameLoftyLagoon()
{

}

void CGameLoftyLagoon::Initialize()
{
	Report->Event( "Initializing Lofty Lagoon.\n" );
}

void CGameLoftyLagoon::Frame()
{
	
}

void CGameLoftyLagoon::Tick()
{
	CRenderer& Renderer = CWindow::GetInstance().GetRenderer();

	const float CurrentTime = static_cast<float>( GameLayersInstance->GetCurrentTime() );

	const float Realm = glm::sin( 0.5f + CurrentTime * 0.5f );
	const float Ralm = glm::abs( Realm ) + 0.1f;

	const float CameraSin = glm::sin( CurrentTime * 0.1f );
	const float CameraCos = glm::cos( CurrentTime * 0.1f );

	static const float WorldSize = 5000.0f;

	CCamera Camera;
	FCameraSetup& CameraSetup = Camera.GetCameraSetup();
	CameraSetup.AspectRatio = CConfiguration::GetInstance().GetFloat( "width" ) / CConfiguration::GetInstance().GetFloat( "height" );

	glm::vec3 CameraPhase = glm::vec3( CameraSin * WorldSize * 0.5f, CameraCos * WorldSize * 0.5f, 0.0f );

	CameraSetup.CameraPosition = glm::vec3( -CameraPhase[0], -CameraPhase[1], 25.0f + ( CameraSin * 0.5f + 0.5f ) * 1000.0f );

	CameraSetup.CameraDirection = glm::normalize( glm::vec3( 0.0f, 1.0f, 0.0f ) );
	CameraSetup.CameraDirection = glm::normalize( CameraPhase - CameraSetup.CameraPosition );

	std::srand( 0 );

	for( int i = 0; i < 500; i++ )
	{
		CRenderable* Renderable = new CRenderable();
		Renderable->SetMesh( Renderer.FindMesh( "pyramid" ) );

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();

		const float RandomOffsetX = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * WorldSize ) - ( WorldSize * 0.5f );
		const float RandomOffsetY = ( ( static_cast<float>( std::rand() ) / RAND_MAX ) * WorldSize ) - ( WorldSize * 0.5f );

		RenderData.Color = glm::vec4( glm::abs( glm::sin( CurrentTime + static_cast<float>( i ) * 0.25f ) ), glm::abs( glm::cos( CurrentTime + static_cast<float>( i ) * 0.33f ) ), glm::abs( glm::sin( CurrentTime * 0.5f + static_cast<float>( i ) * 0.5f ) ), 1.0f );
		RenderData.Position = glm::vec3( RandomOffsetX, RandomOffsetY, ( glm::sin( CurrentTime + RandomOffsetX * 0.001f ) * 0.5f + 0.5f ) * ( glm::cos( CurrentTime + RandomOffsetY * 0.001f ) * 0.5f + 0.5f ) * 500.0f );
		RenderData.Size = glm::vec3( 10.0f, 10.0f, 10.0f );

		Renderer.QueueDynamicRenderable( Renderable );
	}

	Camera.Update();
	Renderer.SetCamera( Camera );
}

void CGameLoftyLagoon::Shutdown()
{
	
}

Version CGameLoftyLagoon::GetVersion() const
{
	static Version Number;
	Number.Major = 0;
	Number.Minor = 0;
	Number.Hot = 0;

	return Number;
}
