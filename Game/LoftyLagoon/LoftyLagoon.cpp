// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LoftyLagoon.h"

#include <Engine/Profiling/Logging.h>

#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Configuration/Configuration.h>
#include <Engine/Utility/Locator/InputLocator.h>

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

	Camera.Update();
	Renderer.SetCamera( Camera );

	std::srand( 0 );

	CMesh* PyramidMesh = Renderer.FindMesh( "pyramid" );
	CShader* PyramidShader = Renderer.FindShader( "pyramidocean" );

	if( PyramidMesh && PyramidShader )
	{
		for( int RenderableIndex = 0; RenderableIndex < 500; RenderableIndex++ )
		{
			CRenderable* Renderable = new CRenderable();
			Renderable->SetMesh( PyramidMesh );
			Renderable->SetShader( PyramidShader );

			FRenderDataInstanced& RenderData = Renderable->GetRenderData();


			const float RandomOffsetX = static_cast<float>( std::rand() ) / RAND_MAX;
			const float RandomOffsetY = static_cast<float>( std::rand() ) / RAND_MAX;

			const float RandomOscillationX = ( RandomOffsetX * WorldSize ) - ( WorldSize * 0.5f );
			const float RandomOscillationY = ( RandomOffsetY * WorldSize ) - ( WorldSize * 0.5f );

			const float WaveSin = glm::sin( CurrentTime * 1.33f + static_cast<float>( RenderableIndex ) * 0.25f );
			const float WaveCos = glm::cos( CurrentTime * 1.33f + static_cast<float>( RenderableIndex ) * 0.25f );

			RenderData.Color = glm::vec4( glm::abs( glm::sin( CurrentTime + static_cast<float>( RenderableIndex ) * 0.25f ) ), glm::abs( glm::cos( CurrentTime + static_cast<float>( RenderableIndex ) * 0.33f ) ), glm::abs( glm::sin( CurrentTime * 0.5f + static_cast<float>( RenderableIndex ) * 0.5f ) ), 1.0f );
			RenderData.Position = glm::vec3( RandomOscillationX + ( WaveSin * -10.0f ), RandomOscillationY + ( WaveCos * -10.0f ), ( glm::sin( CurrentTime + RandomOscillationX * 0.001f ) * 0.5f + 0.5f ) * ( glm::cos( CurrentTime + RandomOscillationY * 0.001f ) * 0.5f + 0.5f ) * 500.0f );
			RenderData.Size = glm::vec3( 10.0f, 10.0f, 10.0f );

			Renderer.QueueDynamicRenderable( Renderable );
		}

		IInput& Input = CInputLocator::GetService();
		FFixedPosition2D MousePosition = Input.GetMousePosition();
		glm::vec3 MousePositionWorldSpace = Renderer.ScreenPositionToWorld( glm::vec2( static_cast<float>( MousePosition.X ), static_cast<float>( MousePosition.Y ) ) );

		const glm::vec3 PlaneOrigin = glm::vec3( 0.0f, 0.0f, 0.0f );
		const glm::vec3 PlaneNormal = glm::vec3( 0.0f, 0.0f, 1.0f );
		const bool bPlaneIntersection = Renderer.PlaneIntersection( MousePositionWorldSpace, CameraSetup.CameraPosition, MousePositionWorldSpace, PlaneOrigin, PlaneNormal );

		if( bPlaneIntersection )
		{
			CRenderable* Renderable = new CRenderable();
			Renderable->SetMesh( PyramidMesh );
			Renderable->SetShader( PyramidShader );

			FRenderDataInstanced& RenderData = Renderable->GetRenderData();

			RenderData.Color = glm::vec4( 0.25f, 1.0f, 0.0f, 1.0f );
			RenderData.Position = MousePositionWorldSpace;
			RenderData.Size = glm::vec3( 10.0f, 10.0f, 10.0f );

			Renderer.QueueDynamicRenderable( Renderable );
		}
	}
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
