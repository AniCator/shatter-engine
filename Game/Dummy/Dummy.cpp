// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Dummy.h"

#include <Engine/Profiling/Profiling.h>
#include <Engine/Profiling/Logging.h>

#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Configuration/Configuration.h>
#include <Engine/Resource/Assets.h>

#include <Engine/Utility/Math.h>
#include <Engine/Utility/Math/SIMDVector.h>

#include <cstdlib>

#include <Engine/Application/Application.h>

#ifdef ConsoleWindowDisabled
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
#endif

CDummyLayer* DummyInstance = new CDummyLayer();

void main()
{
	if( !GameLayersInstance )
	{
		Log::Event( Log::Fatal, "Game layers instance does not exist!\n" );
	}

	GameLayersInstance->Add( DummyInstance );

	CApplication Application;
	Application.SetName( "Dummy" );
	Application.Run();
}

CDummyLayer::CDummyLayer()
{
	
}

CDummyLayer::~CDummyLayer()
{

}

void CDummyLayer::Initialize()
{
	for( int i = 0; i < 1000; i++ )
	{
		TestRenderables.push_back( new CRenderable() );
	}
}

void CDummyLayer::Frame()
{
	
}

void CDummyLayer::Tick()
{
	CRenderer& Renderer = CWindow::Get().GetRenderer();

	CCamera Camera;
	Camera.Update();
	Renderer.SetCamera( Camera );

	const float TimeStamp = static_cast<float>( GameLayersInstance->GetCurrentTime() );
	const float Frequency = TimeStamp;
	const float Amplitude = 1.0f;
	const float SinTime = sin( Frequency * 8.0f ) * Amplitude;
	const float CosTime = cos( Frequency * 8.0f ) * Amplitude;

	int Index = 0;
	for( auto TestRenderable : TestRenderables )
	{
		float Offset = static_cast<float>( Index );
		const float Sin2Time = ( sin( Frequency + Offset ) * Offset * Amplitude );// +sin( ( Frequency + Offset ) * 10 ) * 10;
		const float Cos2Time = cos( Frequency + Offset ) * Offset * Amplitude;

		const bool ValidRenderable = TestRenderable->GetMesh() != nullptr;
		FRenderDataInstanced& RenderData = TestRenderable->GetRenderData();

		if( !ValidRenderable )
		{
			TestRenderable->SetMesh( CAssets::Get().FindMesh( "square" ) );
			RenderData.Transform.SetSize( glm::vec3( 5.0f ) );
			RenderData.Color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
		}
		else
		{
			RenderData.Transform.SetPosition( glm::vec3( Cos2Time, Sin2Time, 100.0f ) );

			const float Distance = glm::clamp( 1.0f - glm::distance( RenderData.Transform.GetPosition(), glm::vec3( 0.0f, 0.0f, 0.0f ) ) * 0.002f, 0.25f, 1.0f );

			RenderData.Color = glm::vec4( Distance, 0.0f, 0.0f, 1.0f );
		}

		Renderer.QueueRenderable( TestRenderable );

		Index++;
	}
}

void CDummyLayer::Shutdown()
{
	TestRenderables.clear();
}

Version CDummyLayer::GetVersion() const
{
	static Version Number;
	Number.Major = 1;
	Number.Minor = 0;
	Number.Hot = 0;

	return Number;
}
