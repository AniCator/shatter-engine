// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderable.h"

CRenderable::CRenderable()
{
	Mesh = nullptr;
}

CRenderable::~CRenderable()
{

}

CMesh* CRenderable::GetMesh()
{
	return Mesh;
}


void CRenderable::SetMesh( CMesh* Mesh )
{
	this->Mesh = Mesh;
}

CShader* CRenderable::GetShader()
{
	return Shader;
}

void CRenderable::SetShader( CShader* Shader )
{
	this->Shader = Shader;
}

void CRenderable::Draw()
{
	Mesh->Draw( RenderData.DrawMode );
}

FRenderDataInstanced& CRenderable::GetRenderData()
{
	return RenderData;
}
