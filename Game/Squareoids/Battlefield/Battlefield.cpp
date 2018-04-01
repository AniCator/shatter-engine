#include "Battlefield.h"

#include <Game/Game.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Renderable.h>

CSquareoidsBattlefield::CSquareoidsBattlefield()
{

}

CSquareoidsBattlefield::~CSquareoidsBattlefield()
{

}

void CSquareoidsBattlefield::Update()
{
	CRenderer& Renderer = CWindow::GetInstance().GetRenderer();
	CMesh* SquareMesh = Renderer.FindMesh( "square" );

	CRenderable* Renderable = new CRenderable();
	Renderable->SetMesh( SquareMesh );

	FRenderDataInstanced& RenderData = Renderable->GetRenderData();

	RenderData.Color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f );
	RenderData.Position = glm::vec3( 0.0f, 0.0f, 100.0f );
	RenderData.Size = glm::vec3( 5.0f, 5.0f, 5.0f );

	Renderer.QueueDynamicRenderable( Renderable );
}
