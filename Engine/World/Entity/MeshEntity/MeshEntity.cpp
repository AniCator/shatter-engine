// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshEntity.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>

#include <Engine/Display/UserInterface.h>

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
	// Spawn( Mesh, Shader, Texture, Transform );
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

	/*if( !this->Shader )
	{
		this->Shader = CAssets::Get().FindShader( "pyramidocean" );
	}*/
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
			MeshName = Property->Value;
		}
		else if( Property->Key == "shader" )
		{
			ShaderName = Property->Value;
		}
		else if( Property->Key == "texture" )
		{
			TextureName = Property->Value;
		}
		else if( Property->Key == "color" )
		{
			size_t ExpectedTokenCount = 4;
			size_t OutTokenCount = 0;
			auto Components = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, ExpectedTokenCount );
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

	Reload();
}

void CMeshEntity::Reload()
{
	CAssets& Assets = CAssets::Get();
	CMesh* Mesh = Assets.FindMesh( MeshName );
	CShader* Shader = Assets.FindShader( ShaderName );
	CTexture* Texture = Assets.FindTexture( TextureName );

	Spawn( Mesh, Shader, Texture, Transform );
}

void CMeshEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );

	FDataString::Decode( Data, MeshName );
	FDataString::Decode( Data, ShaderName );
	FDataString::Decode( Data, TextureName );

	Data >> Color;
}

void CMeshEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );

	FDataString::Encode( Data, MeshName );
	FDataString::Encode( Data, ShaderName );
	FDataString::Encode( Data, TextureName );

	Data << Color;
}
