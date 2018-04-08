// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "TitleScreen.h"

#include <Game/Game.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>

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
	CRenderer& Renderer = CWindow::GetInstance().GetRenderer();

	CMesh* SquareMesh = Renderer.FindMesh( "square" );

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
				RenderData.Position = glm::vec3( OffsetX, OffsetY, 100.0f );
				RenderData.Size = glm::vec3( 5.0f, 5.0f, 5.0f ) * ( 1.0f - Amplitude );

				Renderer.QueueDynamicRenderable( Renderable );
			}
		}
	}

	CCamera Camera;
	FCameraSetup& Setup = Camera.GetCameraSetup();
	Renderer.SetCamera( Camera );
}
