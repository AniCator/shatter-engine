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
	Transform = FTransform();
	Renderable = nullptr;

	Color = glm::vec4( 0.65f, 0.35f, 0.45f, 1.0f );
}

CMeshEntity::CMeshEntity( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform ) : CEntity()
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
	CAssets& Assets = CAssets::Get();

	CMesh* Mesh = nullptr;
	CShader* Shader = nullptr;
	CTexture* Texture = nullptr;
	glm::vec3 Position;
	glm::vec3 Orientation;
	glm::vec3 Size;

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
		else if( Property->Key == "position" )
		{
			const std::vector<float>& Coordinates = ExtractTokensFloat( Property->Value, ' ', 3 );
			if( Coordinates.size() == 3 )
			{
				Position = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "rotation" )
		{
			const std::vector<float>& Coordinates = ExtractTokensFloat( Property->Value, ' ', 3 );
			if( Coordinates.size() == 3 )
			{
				Orientation = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "scale" )
		{
			const std::vector<float>& Coordinates = ExtractTokensFloat( Property->Value, ' ', 3 );
			if( Coordinates.size() == 3 )
			{
				Size = glm::vec3( Coordinates[0], Coordinates[1], Coordinates[2] );
			}
		}
		else if( Property->Key == "color" )
		{
			const std::vector<float>& Components = ExtractTokensFloat( Property->Value, ' ', 4 );
			if( Components.size() == 3 )
			{
				Color = glm::vec4( Components[0], Components[1], Components[2], 1.0f );
			}
			else if( Components.size() == 4 )
			{
				Color = glm::vec4( Components[0], Components[1], Components[2], Components[3] );
			}
		}
	}

	FTransform Transform( Position, Orientation, Size );
	Spawn( Mesh, Shader, Texture, Transform );
}
