// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "MeshEntity.h"

#include <Game/Game.h>

#include <Engine/Animation/Animator.h>
#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Window.h>
#include <Engine/Physics/Physics.h>
#include <Engine/Physics/PhysicsComponent.h>
#include <Engine/Physics/Body/Body.h>
#include <Engine/Physics/Body/Plane.h>
#include <Engine/Profiling/Profiling.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/World.h>

#include <Engine/Display/UserInterface.h>

#include <Engine/World/Entity/LightEntity/LightEntity.h>

static CEntityFactory<CMeshEntity> Factory( "mesh" );

ConfigurationVariable<bool> DisplaySkeleton( "debug.MeshEntity.DisplaySkeleton", false );
ConfigurationVariable<bool> DisplayLightInfluences( "debug.MeshEntity.DisplayLightInfluences", false );

CMeshEntity::CMeshEntity()
{
	Mesh = nullptr;
	CollisionMesh = nullptr;
	Shader = nullptr;
	Textures.reserve( 8 );
	Renderable = nullptr;
	PhysicsBody = nullptr;
	Static = true;
	Stationary = false;
	Contact = false;
	Collision = true;
	Visible = true;

	Color = Vector4D( 1.0f, 1.0f, 1.0f, 1.0f );

	Inputs["PlayAnimation"] = [&] ( CEntity* Origin )
	{
		SetAnimation( AnimationInstance.CurrentAnimation );

		return true;
	};

	Inputs["LoopAnimation"] = [&] ( CEntity* Origin )
	{
		SetAnimation( AnimationInstance.CurrentAnimation, true );

		return true;
	};

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
	AnimationInstance.Mesh = Mesh;
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
		ConstructRenderable();
		ConstructPhysics();
	}

	CPointEntity::Construct();
}

void CMeshEntity::Tick()
{
	if( ShouldUpdateTransform )
	{
		if( Mesh )
		{
			WorldBounds = Math::AABB( Mesh->GetBounds(), Transform );
		}
	}

	bool IsCulled = !IsVisible();
	if( !IsCulled && MaximumRenderDistance > 0.0f )
	{
		if( const auto* Camera = GetWorld()->GetActiveCamera() )
		{
			const auto RenderDistanceSquared = MaximumRenderDistance * MaximumRenderDistance;
			const auto DistanceSquared = Transform.GetPosition().DistanceSquared( Camera->GetCameraPosition() );
			if( DistanceSquared > RenderDistanceSquared )
			{
				IsCulled = true;
			}
		}
	}

	if( !IsCulled && Renderable )
	{
		if( ShouldUpdateTransform )
		{
			GetTransform();
		}

		WantsAnimationUpdate = true;

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();
		RenderData.Transform = Transform;
		RenderData.Color = Color;
		RenderData.WorldBounds = WorldBounds;

		// Set the light origin to the transform position if we're not overriding it.
		if( !UseLightOrigin )
		{
			LightOrigin = RenderData.Transform.GetPosition();
		}

		if( RenderData.LightIndex.Index[0] == -1 || !Static || true )
		{
			RenderData.LightIndex = LightEntity::Fetch( LightOrigin );
		}
	}

	CPointEntity::Tick();
}

void PrintColumn( const Vector4D& Column )
{
	Log::Event( "%.3f %.3f %.3f %.3f\n", Column.X, Column.Y, Column.Z, Column.W );
}

void PrintMatrix( const std::string& Name, const Matrix4D& Matrix )
{
	Log::Event( "%s\n", Name.c_str() );
	PrintColumn( Matrix.Columns[0] );
	PrintColumn( Matrix.Columns[1] );
	PrintColumn( Matrix.Columns[2] );
	PrintColumn( Matrix.Columns[3] );
}

void CMeshEntity::TickAnimation()
{
	if( !Renderable->GetRenderData().ShouldRender )
		return; // Mesh is invisible.

	// Profile( "Animation" );

	AnimationInstance.TickOffset = GetEntityID().ID;

	Animator::Update( AnimationInstance, DeltaTime, ForceAnimationTick );

	if( ForceAnimationTick )
	{
		ForceAnimationTick = false;
	}

	if( AnimationInstance.Bones.empty() )
		return;

	const auto WorldTransform = GetTransform();
	static auto NewBoundVertices = std::vector<Vector3D>();
	NewBoundVertices.clear();
	for( size_t MatrixIndex = 0; MatrixIndex < AnimationInstance.Bones.size(); MatrixIndex++ )
	{
		auto Matrix = AnimationInstance.Bones[MatrixIndex].GlobalTransform;
		auto ParentMatrix = Matrix;
		if( AnimationInstance.Bones[MatrixIndex].ParentIndex > -1 )
		{
			ParentMatrix = AnimationInstance.Bones[AnimationInstance.Bones[MatrixIndex].ParentIndex].GlobalTransform;
		}

		const auto Current = Matrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
		Vector3D PointTarget = WorldTransform.Transform( Current );
		NewBoundVertices.emplace_back( PointTarget );
		
		if( DisplaySkeleton && IsDebugEnabled() )
		{
			const auto Parent = ParentMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
			Vector3D PointSource = WorldTransform.Transform( Parent );

			Vector3D PointCenter = PointSource + ( PointTarget - PointSource ) * 0.5f;

			const auto Bind = AnimationInstance.Bones[MatrixIndex].BoneToModel.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
			Vector3D PointBind = WorldTransform.Transform( Bind );

			auto ParentBindMatrix = AnimationInstance.Bones[MatrixIndex].BoneToModel;
			if( AnimationInstance.Bones[MatrixIndex].ParentIndex > -1 )
			{
				ParentBindMatrix = AnimationInstance.Bones[AnimationInstance.Bones[MatrixIndex].ParentIndex].BoneToModel;
			}

			const auto ParentBind = ParentBindMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
			Vector3D PointParentBind = WorldTransform.Transform( ParentBind );
			
			UI::AddCircle( PointSource, 3.0f, ::Color( 0, 0, 255 ) );
			UI::AddLine( PointSource, PointCenter, ::Color( 0, 0, 255 ) );
			UI::AddLine( PointCenter, PointTarget, ::Color( 255, 0, 0 ) );
			UI::AddCircle( PointTarget, 3.0f, ::Color( 255, 0, 0 ) );

			if( Mesh )
			{
				UI::AddText( PointTarget, Mesh->GetSkeleton().MatrixNames[MatrixIndex].c_str() );
			}

			UI::AddCircle( PointParentBind, 3.0f, ::Color( 0, 255, 255 ) );
			UI::AddLine( PointParentBind, PointBind, ::Color( 255, 255, 0 ) );
			UI::AddCircle( PointBind, 3.0f, ::Color( 0, 255, 255 ) );
		}
	}

	WorldBounds = Math::AABB( NewBoundVertices.data(), NewBoundVertices.size() );
	FRenderDataInstanced& RenderData = Renderable->GetRenderData();
	RenderData.WorldBounds = WorldBounds;

	UseLightOrigin = true;
	LightOrigin = RenderData.WorldBounds.Center();
}

void CMeshEntity::Frame()
{
	if( WantsAnimationUpdate )
	{
		TickAnimation();
		WantsAnimationUpdate = false;
	}

	bool IsCulled = !IsVisible();
	if( !IsCulled && MaximumRenderDistance > 0.0f )
	{
		if( const auto* Camera = GetWorld()->GetActiveCamera() )
		{
			const auto RenderDistanceSquared = MaximumRenderDistance * MaximumRenderDistance;
			const auto DistanceSquared = Transform.GetPosition().DistanceSquared( Camera->GetCameraPosition() );
			if( DistanceSquared > RenderDistanceSquared )
			{
				IsCulled = true;
			}
		}
	}

	if( IsCulled || !Renderable )
		return;

	Animator::Submit( AnimationInstance, Renderable );
	QueueRenderable( Renderable );
}

void CMeshEntity::Destroy()
{
	if( PhysicsBody )
	{
		PhysicsBody->Destroy();
		PhysicsBody = nullptr;
	}

	CPointEntity::Destroy();
}

void DebugLight( const Vector3D& Position, const int32_t& Index )
{
	if( Index < 0 )
		return;

	const auto& Light = LightEntity::Get( Index );
	const auto LightPosition = Vector3D( Light.Position.x, Light.Position.y, Light.Position.z );
	UI::AddLine( Position, LightPosition, Color::Purple );

	const auto DebugColor = ::Color( Light.Color.r * 255.0f, Light.Color.g * 255.0f, Light.Color.b * 255.0f );
	UI::AddCircle( LightPosition, Math::Max( 10.0f, Light.Direction.w ), DebugColor );

	Vector3D Direction;
	Direction.X = Light.Direction.x;
	Direction.Y = Light.Direction.y;
	Direction.Z = Light.Direction.z;

	auto Unit = ( Position - LightPosition );
	const auto Length = Unit.Normalize();
	const auto Angle = cosf( Light.Properties.y );
	const auto Visibility = Unit.Dot( Direction ) - Angle;

	UI::AddText( LightPosition, "Visibility", Visibility );
	UI::AddText( LightPosition + Vector3D( 0.0f, 0.0f, 1.0f ), "Angle", Angle );
}

void CMeshEntity::Debug()
{
	CPointEntity::Debug();

	const auto Position = Transform.GetPosition();
	const auto ForwardVector = Transform.Rotate( Vector3D( 0.0f, 1.0f, 0.0f ) ).Normalized();
	const auto RightVector = ForwardVector.Cross( WorldUp );
	const auto UpVector = RightVector.Cross( ForwardVector );

	UI::AddLine( Position, Position + ForwardVector, Color::Green );
	UI::AddLine( Position, Position + RightVector, Color::Red );
	UI::AddLine( Position, Position + UpVector, Color::Blue );

	if( Mesh )
	{
		UI::AddAABB( WorldBounds.Minimum, WorldBounds.Maximum, Collision ? ( Contact ? Color::Red : Color::Blue ) : Color::Black );

		if( PhysicsBody )
		{
			PhysicsBody->Debug();
		}
	}

	if( DisplayLightInfluences && Renderable )
	{
		const auto& RenderData = Renderable->GetRenderData();
		DebugLight( Position, RenderData.LightIndex.Index[0] );
		DebugLight( Position, RenderData.LightIndex.Index[1] );
		DebugLight( Position, RenderData.LightIndex.Index[2] );
		DebugLight( Position, RenderData.LightIndex.Index[3] );
	}
}

void CMeshEntity::Load( const JSON::Vector& Objects )
{
	CPointEntity::Load( Objects );
	CAssets& Assets = CAssets::Get();

	TextureNames.clear();

	for( auto* Property : Objects )
	{
		if( Property->Key == "mesh" )
		{
			MeshName = Property->Value;
		}
		else if( Property->Key == "collisionmesh" )
		{
			CollisionMeshName = Property->Value;
		}
		else if( Property->Key == "collisiontype" )
		{
			CollisionType = ToBodyType( Property->Value );
		}
		else if( Property->Key == "project" )
		{
			ShouldProject = Property->Value == "0" ? false : true;
		}
		else if( Property->Key == "shader" )
		{
			ShaderName = Property->Value;
			std::transform( ShaderName.begin(), ShaderName.end(), ShaderName.begin(), ::tolower );
		}
		else if( Property->Key == "texture" )
		{
			if( !Property->Objects.empty() )
			{
				for( auto* TextureProperty : Property->Objects )
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
			const size_t ExpectedTokenCount = 4;
			size_t OutTokenCount = 0;
			auto* Components = ExtractTokensFloat( Property->Value.c_str(), ' ', OutTokenCount, ExpectedTokenCount );
			if( OutTokenCount == 3 )
			{
				Color = Vector4D( Components[0], Components[1], Components[2], 1.0f );
			}
			else if( OutTokenCount == 4 )
			{
				Color = Vector4D( Components[0], Components[1], Components[2], Components[3] );
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
		else if( Property->Key == "visible" )
		{
			if( Property->Value == "0" )
			{
				Visible = false;
			}
			else
			{
				Visible = true;
			}
		}
		else if( Property->Key == "animation" && Property->Value.length() > 0 )
		{
			AnimationInstance.CurrentAnimation = Property->Value;
		}
		else if( Property->Key == "playrate" && Property->Value.length() > 0 )
		{
			AnimationInstance.PlayRate = ParseFloat( Property->Value.c_str() );
		}
		else if( Property->Key == "loop" && Property->Value.length() > 0 )
		{
			Extract( Property->Value, AnimationInstance.LoopAnimation );
		}
		else if( Property->Key == "maximum_render_distance" && Property->Value.length() > 0 )
		{
			Extract( Property->Value, MaximumRenderDistance );
		}
	}

	if( TextureNames.empty() )
	{
		TextureNames.emplace_back( "error" );
	}

	Reload();
}

void CMeshEntity::Reload()
{
	CAssets& Assets = CAssets::Get();
	CMesh* TargetMesh = Assets.Meshes.Find( MeshName );
	CShader* TargetShader = Assets.Shaders.Find( ShaderName );

	if( !TargetShader )
	{
		// Try to use the default shader if the specified shader can't be found.
		TargetShader = Assets.Shaders.Find( "default" );
	}

	Spawn( TargetMesh, TargetShader, nullptr, Transform );

	CMesh* TargetCollisionMesh = Assets.Meshes.Find( CollisionMeshName );
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

	std::string CollisionTypeString;
	DataString::Decode( Data, CollisionTypeString );
	CollisionType = ToBodyType( CollisionTypeString );

	Data >> Static;
	Data >> Stationary;
	Data >> MaximumRenderDistance;
}

void CMeshEntity::Export( CData& Data )
{
	CPointEntity::Export( Data );

	DataString::Encode( Data, MeshName );
	DataString::Encode( Data, CollisionMeshName );
	DataString::Encode( Data, ShaderName );
	
	size_t Size = TextureNames.size();
	Data << Size;

	for( const auto& TextureName : TextureNames )
	{
		DataString::Encode( Data, TextureName );
	}

	Data << Color;
	Data << Collision;
	Data << Visible;
	DataString::Encode( Data, FromBodyType( CollisionType ) );

	Data << Static;
	Data << Stationary;
	Data << MaximumRenderDistance;
}

bool CMeshEntity::ShouldCollide() const
{
	return Collision;
}

void CMeshEntity::SetCollision( const bool Enable )
{
	Collision = Enable;
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

BoundingBox CMeshEntity::GetWorldBounds() const
{
	return WorldBounds;
}

CBody* CMeshEntity::GetBody() const
{
	return PhysicsBody;
}

const FTransform& CMeshEntity::GetTransform()
{
	const bool UpdateBounds = ShouldUpdateTransform;
	CPointEntity::GetTransform();
	
	if( UpdateBounds && Mesh )
	{
		WorldBounds = Math::AABB( Mesh->GetBounds(), Transform );
	}
	
	return WorldTransform;
}

void CMeshEntity::SetAnimation( const std::string& Name, const bool& Loop )
{
	AnimationInstance.SetAnimation( Name, Loop );
	ForceAnimationTick = true;
}

const std::string& CMeshEntity::GetAnimation() const
{
	return AnimationInstance.GetAnimation();
}

bool CMeshEntity::HasAnimation( const std::string& Name ) const
{
	return AnimationInstance.HasAnimation( Name );
}

bool CMeshEntity::IsAnimationFinished() const
{
	return AnimationInstance.IsAnimationFinished();
}

float CMeshEntity::GetPlayRate() const
{
	return AnimationInstance.GetPlayRate();
}

void CMeshEntity::SetPlayRate( const float& PlayRate )
{
	AnimationInstance.SetPlayRate( PlayRate );
}

float CMeshEntity::GetAnimationTime() const
{
	return AnimationInstance.GetAnimationTime();
}

void CMeshEntity::SetAnimationTime( const float& Value )
{
	AnimationInstance.SetAnimationTime( Value );
}

void CMeshEntity::SetPosition( const Vector3D& Position, const bool& Teleport )
{
	ShouldUpdateTransform = true;

	Transform.SetPosition( Position );

	if( Teleport && PhysicsBody )
	{
		PhysicsBody->PreviousTransform = Transform;
	}
}

void CMeshEntity::SetOrientation( const Vector3D& Orientation )
{
	ShouldUpdateTransform = true;

	Transform.SetOrientation( Orientation );

	if( PhysicsBody )
	{
		PhysicsBody->PreviousTransform = Transform;
	}
}

void CMeshEntity::SetSize( const Vector3D& Size )
{
	ShouldUpdateTransform = true;

	Transform.SetSize( Size );

	if( PhysicsBody )
	{
		PhysicsBody->PreviousTransform = Transform;
	}
}

void CMeshEntity::ConstructRenderable()
{
	if( !Mesh )
		return;

	Renderable = new CRenderable();
	Renderable->SetMesh( Mesh );

	if( Shader )
	{
		Renderable->SetShader( Shader );
	}

	if( !Textures.empty() )
	{
		size_t Index = 0;
		for( auto* Texture : Textures )
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
	RenderData.WorldBounds = WorldBounds;

	const auto& Set = Mesh->GetAnimationSet();
	const auto& Skeleton = Set.Skeleton;
	if( !Skeleton.Bones.empty() && !Skeleton.Animations.empty() )
	{
		const auto FirstAnimation = ( *Skeleton.Animations.begin() ).first;
		SetAnimation( FirstAnimation, AnimationInstance.LoopAnimation );
	}

	AnimationInstance.Mesh = Mesh;
}

void CMeshEntity::ConstructPhysics()
{
	if( !Mesh )
		return;

	if( !Collision )
		return;

	auto* World = GetWorld();
	if( !World )
		return;

	auto* Physics = World->GetPhysics();
	if( !Physics )
		return;

	if( PhysicsBody )
	{
		PhysicsBody->Destroy();
		delete PhysicsBody;
	}

	if( CollisionType == BodyType::Plane )
	{
		auto* PlaneBody = new CPlaneBody();
		PlaneBody->TwoSidedCollision = true;
		PlaneBody->ProjectToSurface = ShouldProject;
		PhysicsBody = PlaneBody;
	}
	else if( CollisionType == BodyType::AABB )
	{
		PhysicsBody = new CBody();
	}
	else
	{
		PhysicsBody = new CBody();
		PhysicsBody->TriangleMesh = true;
	}

	PhysicsBody->Owner = this;
	PhysicsBody->Static = Static;
	PhysicsBody->Stationary = Stationary;
	PhysicsBody->Block = true;
	PhysicsBody->LocalBounds = Mesh->GetBounds();
	PhysicsBody->Construct( Physics );
}

void CMeshEntity::QueueRenderable( CRenderable* Renderable )
{
	CRenderer& Renderer = CWindow::Get().GetRenderer();
	Renderer.QueueRenderable( Renderable );
}
