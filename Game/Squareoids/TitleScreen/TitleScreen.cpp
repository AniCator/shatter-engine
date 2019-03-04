// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TitleScreen.h"

#include <Game/Game.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Utility/Locator/InputLocator.h>
#include <glm/gtc/matrix_transform.hpp>

CSquareoidsTitleScreen::CSquareoidsTitleScreen()
{

}

CSquareoidsTitleScreen::~CSquareoidsTitleScreen()
{

}

static const int MaxSquaresX = 50;
static const int MaxSquaresY = 8;
static const int SquaroidsSquareName[MaxSquaresY][MaxSquaresX] = {
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ 0,0,1,1,1,0,0,0,1,1,1,0,0,1,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,0,0,0,1,0,0,0,0,0,0 },
	{ 0,1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,0,1,1,0,0 },
	{ 0,0,1,1,0,0,0,1,0,0,0,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,1,1,1,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0 },
	{ 0,0,0,0,1,0,0,1,0,1,0,1,0,1,0,0,1,0,0,1,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,1,0,1,1,1,0,0,1,0,0,0 },
	{ 0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,1,0,0,0,0,1,0,0,1,0,1,0,1,0,1,0,0,0,1,0,0 },
	{ 0,1,1,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,1,0,1,0,0,1,0,1,1,1,1,0,0,1,1,0,0,1,0,1,1,1,0,1,1,0,0,0 },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
};

void CSquareoidsTitleScreen::Display()
{
	CRenderer& Renderer = CWindow::Get().GetRenderer();

	CMesh* SquareMesh = CAssets::Get().FindMesh( "square" );

	const float Distance = 20.0f;
	float StartPositionX = Distance * -MaxSquaresX * 0.5f;
	float StartPositionY = Distance * -MaxSquaresY * 0.5f;
	const float StartOffsetY = StartPositionY;

	StartPositionY -= StartOffsetY * 2.0f;

	const float Time = static_cast<float>( GameLayersInstance->GetCurrentTime() ) + 0.5f;
	const float Frequency = Time * 4.0f;
	const float Amplitude = pow( ( sin( Time ) * 0.5f + 0.5f ), 2 ) * ( cos( Time ) * 0.5f + 0.5f );
	const float AmplitudeScaledX = Amplitude * 50.0f;
	const float AmplitudeScaledY = Amplitude * 50.0f;

	for( int Iterations = 0; Iterations < 3; Iterations++ )
	{
		if( Iterations != 0 )
		{
			StartPositionY += StartOffsetY * 2.0f;
		}

		for( int IndexY = 0; IndexY < MaxSquaresY; IndexY++ )
		{
			for( int IndexX = 0; IndexX < MaxSquaresX; IndexX++ )
			{
				CRenderable* Renderable = new CRenderable();
				Renderable->SetMesh( SquareMesh );

				const float OffsetX = -StartPositionX + static_cast<float>( -IndexX ) * Distance + sin( Frequency + IndexX ) * AmplitudeScaledX;
				const float OffsetY = -StartPositionY + static_cast<float>( -IndexY ) * Distance + cos( Frequency + IndexY ) * AmplitudeScaledY;

				const bool VisibleSquare = SquaroidsSquareName[IndexY][IndexX] > 0;

				FRenderDataInstanced& RenderData = Renderable->GetRenderData();

				RenderData.Color = glm::vec4( VisibleSquare, 0.0f, !VisibleSquare, 1.0f );
				RenderData.Transform = { glm::vec3( OffsetX, OffsetY, 0.0f ), WorldUp, glm::vec3( 5.0f ) * ( 1.0f - Amplitude ) };

				Renderer.QueueDynamicRenderable( Renderable );
			}
		}
	}

	CCamera Camera;

	FCameraSetup& CameraSetup = Camera.GetCameraSetup();

	CameraSetup.CameraPosition[0] = 0.0f;
	CameraSetup.CameraPosition[1] = -100.0f;
	CameraSetup.CameraPosition[2] = -600.0f;
	CameraSetup.CameraDirection = glm::normalize( CameraSetup.CameraPosition );

	Camera.Update();
	Renderer.SetCamera( Camera );

	/*// Cursor square
	CRenderable* Renderable = new CRenderable();
	Renderable->SetMesh( SquareMesh );

	FRenderDataInstanced& RenderData = Renderable->GetRenderData();

	IInput& Input = CInputLocator::GetService();
	FFixedPosition2D MousePosition = Input.GetMousePosition();

	glm::mat4& ProjectionInverse = glm::inverse( Camera.GetProjectionMatrix() );
	glm::mat4& ViewInverse = glm::inverse( Camera.GetViewMatrix() );
	float NormalizedMouseX = ( 2.0f * MousePosition.X ) / 1920.0f - 1.0f;
	float NormalizedMouseY = 1.0f - ( 2.0f * MousePosition.Y ) / 1080.0f;
	glm::vec4 MousePositionClipSpace = glm::vec4( NormalizedMouseX, NormalizedMouseY, -1.0f, 1.0f );
	glm::vec4 MousePositionEyeSpace = ProjectionInverse * MousePositionClipSpace;

	// Un-project Z and W
	MousePositionEyeSpace[2] = -1.0f;
	MousePositionEyeSpace[3] = 1.0f;

	glm::vec3 MousePositionWorldSpace = ViewInverse * MousePositionEyeSpace;
	glm::vec3 MouseDirection = glm::normalize( Camera.GetCameraSetup().CameraPosition - MousePositionWorldSpace );

	bool RayCast = false;
	glm::vec3 StartPosition = Camera.GetCameraSetup().CameraPosition;
	float CastDelta = -0.1f;

	glm::vec3 RayCastResult = StartPosition;

	while( !RayCast )
	{
		StartPosition += MouseDirection * CastDelta;
		const float Delta = 100.0f - StartPosition[2];

		if( fabs( Delta ) < 0.5f )
		{
			RayCastResult = StartPosition;
			RayCast = true;
		}
	}

	RenderData.Color = glm::vec4( 1.0f, 0.5f, 0.0f, 1.0f );
	RenderData.Position = RayCastResult;
	RenderData.Size = glm::vec3( 10.0f, 10.0f, 10.0f );

	Renderer.QueueDynamicRenderable( Renderable );*/
}
