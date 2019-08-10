// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshEntity.h"

#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Window.h>
#include <Engine/Physics/Physics.h>
#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/World.h>

#include <Engine/Display/UserInterface.h>

static CEntityFactory<CMeshEntity> Factory( "mesh" );

CMeshEntity::CMeshEntity()
{
	Mesh = nullptr;
	Shader = nullptr;
	Textures.reserve( 8 );
	Renderable = nullptr;
	PhysicsComponent = nullptr;
	Static = true;
	Contact = false;
	Collision = true;

	Color = glm::vec4( 0.65f, 0.35f, 0.45f, 1.0f );
}

CMeshEntity::CMeshEntity( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform ) : CPointEntity()
{
	
}

CMeshEntity::~CMeshEntity()
{

}

void CMeshEntity::Spawn( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform )
{
	this->Mesh = Mesh;
	this->Shader = Shader;
	Textures.clear();
	Textures.emplace_back( Texture );
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

		if( Textures.size() > 0 )
		{
			size_t Index = 0;
			for( auto Texture : Textures )
			{
				if( Texture )
				{
					Renderable->SetTexture( Texture, static_cast<ETextureSlot>( Index ) );
				}

				Index++;

				if( Index >= static_cast<size_t>( ETextureSlot::Maximum ) )
				{
					break;
				}
			}
		}

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();
		RenderData.Transform = Transform;
		RenderData.Color = Color;

		auto World = GetWorld();
		if( World && Collision )
		{
			auto Physics = World->GetPhysics();
			if( Physics )
			{
				if( !PhysicsComponent )
				{
					PhysicsComponent = new CPhysicsComponent( this );
					PhysicsComponent->Construct( Physics );
				}
			}
		}
	}
}

void CMeshEntity::Tick()
{
	if( Renderable )
	{
		if( ShouldUpdateTransform )
		{
			GetTransform();
		}

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();
		RenderData.Transform = Transform;
		RenderData.Color = Color;

		CRenderer& Renderer = CWindow::Get().GetRenderer();
		Renderer.QueueRenderable( Renderable );
	}

	if( Mesh )
	{
		auto& AABB = Mesh->GetBounds();

		FTransform MinimumTransform = FTransform( AABB.Minimum, Vector3D( 0.0f, 0.0f, 0.0f ), Vector3D( 1.0f, 1.0f, 1.0f ) );
		FTransform MaximumTransform = FTransform( AABB.Maximum, Vector3D( 0.0f, 0.0f, 0.0f ), Vector3D( 1.0f, 1.0f, 1.0f ) );

		auto Minimum = Transform.Transform( AABB.Minimum );
		auto Maximum = Transform.Transform( AABB.Maximum );

		WorldBounds.Minimum = Minimum;
		WorldBounds.Maximum = Maximum;
	}
}

void CMeshEntity::Destroy()
{
	if( PhysicsComponent )
	{
		PhysicsComponent->Destroy( GetWorld()->GetPhysics() );
		delete PhysicsComponent;
		PhysicsComponent = nullptr;
	}
}

void CMeshEntity::Debug()
{
	CPointEntity::Debug();

	if( Mesh )
	{
		auto& AABB = Mesh->GetBounds();
		
		// auto Minimum = Transform.Transform( Math::ToGLM( AABB.Minimum ) );
		// auto Maximum = Transform.Transform( Math::ToGLM( AABB.Maximum ) );

		FTransform MinimumTransform = FTransform( AABB.Minimum, Vector3D( 0.0f, 0.0f, 0.0f ), Vector3D( 1.0f, 1.0f, 1.0f ) );
		FTransform MaximumTransform = FTransform( AABB.Maximum, Vector3D( 0.0f, 0.0f, 0.0f ), Vector3D( 1.0f, 1.0f, 1.0f ) );

		auto Minimum = Transform.Transform( AABB.Minimum );
		auto Maximum = Transform.Transform( AABB.Maximum );

		UI::AddAABB( Minimum, Maximum, Collision ? ( Contact ? Color::Red : Color::Blue ) : Color::Black );

		if( PhysicsComponent )
		{
			// UI::AddLine( Transform.GetPosition(), Transform.GetPosition() + PhysicsComponent->Velocity );
		}
	}
}

void CMeshEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );
	CAssets& Assets = CAssets::Get();

	CMesh* Mesh = nullptr;
	CShader* Shader = nullptr;
	CTexture* Texture = nullptr;

	TextureNames.clear();

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
			if( Property->Objects.size() > 0 )
			{
				for( auto TextureProperty : Property->Objects )
				{
					TextureNames.emplace_back( TextureProperty->Value );
				}
			}
			else
			{
				TextureNames.emplace_back( Property->Value );
			}
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
		else if( Property->Key == "collision" )
		{
			if( Property->Value == "0" )
			{
				Collision = false;
			}
			else
			{
				Collision = true;
			}
		}
		else if( Property->Key == "static" )
		{
			if( Property->Value == "0" )
			{
				Static = false;
			}
			else
			{
				Static = true;
			}
		}
	}

	if( TextureNames.size() == 0 )
	{
		TextureNames.emplace_back( "error" );
	}

	Reload();
}

void CMeshEntity::Reload()
{
	CAssets& Assets = CAssets::Get();
	CMesh* Mesh = Assets.FindMesh( MeshName );
	CShader* Shader = Assets.FindShader( ShaderName );

	Spawn( Mesh, Shader, nullptr, Transform );

	Textures.clear();
	for( auto TextureName : TextureNames )
	{
		Textures.emplace_back( Assets.FindTexture( TextureName ) );
	}
}

void CMeshEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );

	FDataString::Decode( Data, MeshName );
	FDataString::Decode( Data, ShaderName );

	size_t Size = 0;
	Data >> Size;
	for( size_t Index = 0; Index < Size; Index++ )
	{
		TextureNames.emplace_back();
		FDataString::Decode( Data, TextureNames.back() );
	}

	Data >> Color;
	Data >> Collision;
}

void CMeshEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );

	FDataString::Encode( Data, MeshName );
	FDataString::Encode( Data, ShaderName );
	
	size_t Size = TextureNames.size();
	Data << Size;

	for( auto TextureName : TextureNames )
	{
		FDataString::Encode( Data, TextureName );
	}

	Data << Color;
	Data << Collision;
}

bool CMeshEntity::IsStatic() const
{
	return Static;
}
