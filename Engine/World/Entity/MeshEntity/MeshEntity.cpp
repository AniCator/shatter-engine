// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshEntity.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>

static CEntityFactory<CMeshEntity> Factory( "mesh" );

CMeshEntity::CMeshEntity()
{
	Mesh = nullptr;
	Shader = nullptr;
	Texture = nullptr;
	Renderable = nullptr;

	Color = glm::vec4( 0.65f, 0.35f, 0.45f, 1.0f );
}

CMeshEntity::CMeshEntity( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform ) : CPointEntity()
{
	Spawn( Mesh, Shader, Texture, Transform );
}

CMeshEntity::~CMeshEntity()
{

}

void CMeshEntity::Spawn( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform )
{
	this->Mesh = Mesh;
	this->Shader = Shader;
	this->Texture = Texture;
	this->Transform = Transform;
}

void CMeshEntity::Construct()
{
	if( Mesh )
	{
		Renderable = new CRenderable();
		Renderable->SetMesh( Mesh );

		if( Shader )
		{
			Renderable->SetShader( Shader );
		}

		if( Texture )
		{
			Renderable->SetTexture( Texture, ETextureSlot::Slot0 );
		}

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();
		RenderData.Transform = Transform;
		RenderData.Color = Color;
	}
}

void CMeshEntity::Tick()
{
	if( Renderable )
	{
		CRenderer& Renderer = CWindow::Get().GetRenderer();
		Renderer.QueueRenderable( Renderable );
	}
}

void CMeshEntity::Destroy()
{

}

void CMeshEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );
	CAssets& Assets = CAssets::Get();

	CMesh* Mesh = nullptr;
	CShader* Shader = nullptr;
	CTexture* Texture = nullptr;

	for( auto Property : Objects )
	{
		if( Property->Key == "mesh" )
		{
			Mesh = Assets.FindMesh( Property->Value );
		}
		else if( Property->Key == "shader" )
		{
			Shader = Assets.FindShader( Property->Value );
		}
		else if( Property->Key == "texture" )
		{
			Texture = Assets.FindTexture( Property->Value );
		}
		else if( Property->Key == "color" )
		{
			size_t ExpectedTokenCount = 4;
			size_t OutTokenCount = 0;
			auto Components = ExtractTokensFloat( Property->Value, ' ', OutTokenCount, ExpectedTokenCount );
			if( OutTokenCount == 3 )
			{
				Color = glm::vec4( Components[0], Components[1], Components[2], 1.0f );
			}
			else if( OutTokenCount == 4 )
			{
				Color = glm::vec4( Components[0], Components[1], Components[2], Components[3] );
			}
		}
	}

	Spawn( Mesh, Shader, Texture, Transform );
}
