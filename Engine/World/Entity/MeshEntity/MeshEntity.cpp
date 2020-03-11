// Copyright � 2017, Christiaan Bakker, All rights reserved.
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
	CollisionMesh = nullptr;
	Shader = nullptr;
	Textures.reserve( 8 );
	Renderable = nullptr;
	PhysicsComponent = nullptr;
	Static = true;
	Stationary = true;
	Contact = false;
	Collision = true;
	Visible = true;

	// Color = glm::vec4( 0.65f, 0.35f, 0.45f, 1.0f );
	Color = glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f );
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
	ShouldUpdateTransform = true;
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
					PhysicsComponent = new CBody( this, CollisionMesh ? CollisionMesh->GetBounds() : Mesh->GetBounds(), Static, Stationary );
					PhysicsComponent->Construct( Physics );
				}
			}
		}
	}
}

void CMeshEntity::Tick()
{
	if( ShouldUpdateTransform )
	{
		if( Mesh )
		{
			FBounds TransformedAABB = Math::AABB( Mesh->GetBounds(), Transform );

			WorldBounds.Minimum = TransformedAABB.Minimum;
			WorldBounds.Maximum = TransformedAABB.Maximum;
		}
	}

	if( IsVisible() && Renderable )
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
}

void CMeshEntity::Destroy()
{
	if( PhysicsComponent )
	{
		PhysicsComponent->Destroy( GetWorld()->GetPhysics() );
		delete PhysicsComponent;
		PhysicsComponent = nullptr;
	}

	CPointEntity::Destroy();
}

void CMeshEntity::Debug()
{
	CPointEntity::Debug();

	if( Mesh )
	{
		UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, Collision ? ( Contact ? Color::Red : Color::Blue ) : Color::Black );

		if( PhysicsComponent )
		{
			PhysicsComponent->Debug();
		}

		auto& Skeleton = Mesh->GetSkeleton();
		if( Skeleton.Matrices.size() > 0 )
		{
			auto WorldTransform = GetTransform();
			for( auto& Matrix : Skeleton.Matrices )
			{
				Vector3D Start = WorldTransform.Transform( Matrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) ) );
				Vector3D End = WorldTransform.Transform( Matrix.Transform( Vector3D( 0.0f, 0.0f, -1.0f ) ) );
				UI::AddLine( Start, End, ::Color( 255, 0, 255 ) );
			}
		}
	}
}

void CMeshEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );
	CAssets& Assets = CAssets::Get();

	TextureNames.clear();

	for( auto Property : Objects )
	{
		if( Property->Key == "mesh" )
		{
			MeshName = Property->Value;
		}
		else if( Property->Key == "collisionmesh" )
		{
			CollisionMeshName = Property->Value;
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
		else if( Property->Key == "stationary" )
		{
			if( Property->Value == "0" )
			{
				Stationary = false;
			}
			else
			{
				Stationary = true;
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
	CMesh* TargetMesh = Assets.FindMesh( MeshName );
	CShader* TargetShader = Assets.FindShader( ShaderName );

	Spawn( TargetMesh, TargetShader, nullptr, Transform );

	CMesh* TargetCollisionMesh = Assets.FindMesh( CollisionMeshName );
	if( TargetCollisionMesh )
	{
		CollisionMesh = TargetCollisionMesh;
	}

	Textures.clear();
	for( auto TextureName : TextureNames )
	{
		Textures.emplace_back( Assets.FindTexture( TextureName ) );
	}
}

void CMeshEntity::Import( CData& Data )
{
	CPointEntity::Import( Data );

	DataString::Decode( Data, MeshName );
	DataString::Decode( Data, CollisionMeshName );
	DataString::Decode( Data, ShaderName );

	size_t Size = 0;
	Data >> Size;
	for( size_t Index = 0; Index < Size; Index++ )
	{
		TextureNames.emplace_back();
		DataString::Decode( Data, TextureNames.back() );
	}

	Data >> Color;
	Data >> Collision;
	Data >> Visible;
}

void CMeshEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );

	DataString::Encode( Data, MeshName );
	DataString::Encode( Data, CollisionMeshName );
	DataString::Encode( Data, ShaderName );
	
	size_t Size = TextureNames.size();
	Data << Size;

	for( auto TextureName : TextureNames )
	{
		DataString::Encode( Data, TextureName );
	}

	Data << Color;
	Data << Collision;
	Data << Visible;
}

bool CMeshEntity::ShouldCollide() const
{
	return Collision;
}

bool CMeshEntity::IsStatic() const
{
	return Static;
}

bool CMeshEntity::IsStationary() const
{
	return Stationary;
}

bool CMeshEntity::IsVisible() const
{
	return Visible;
}

void CMeshEntity::SetVisible( const bool& VisibleIn )
{
	Visible = VisibleIn;
}
