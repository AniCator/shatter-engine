// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshEntity.h"

#include <Game/Game.h>

#include <Engine/Animation/Animator.h>
#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Rendering/Material.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Window.h>
#include <Engine/Physics/Physics.h>
#include <Engine/World/Interactable.h>
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
ConfigurationVariable<bool> DisplayNormals( "debug.MeshEntity.DisplayNormals", false );

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
	Contact = true;
	Collision = true;
	Visible = true;

	Color = Vector4D( 1.0f, 1.0f, 1.0f, 1.0f );

	Inputs["PlayAnimation"] = [this] ( CEntity* Origin )
	{
		SetAnimation( AnimationInstance.CurrentAnimation );

		return true;
	};

	Inputs["LoopAnimation"] = [this] ( CEntity* Origin )
	{
		SetAnimation( AnimationInstance.CurrentAnimation, true );

		return true;
	};

	Inputs["Enable"] = [this]( CEntity* Origin )
	{
		SetVisible( true );

		Send( "OnEnable" );
		return true;
	};

	Inputs["Disable"] = [this]( CEntity* Origin )
	{
		SetVisible( false );

		Send( "OnDisable" );
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
	const auto* World = GetWorld();

	// Find the parent if we haven't yet.
	if( ParentName.length() > 0 && !Parent && World )
	{
		if( auto* Entity = World->Find( ParentName ) )
		{
			SetParent( Entity );
			ShouldUpdateTransform = true;
		}
	}

	// Update the transform data.
	GetTransform();

	if( Mesh )
	{
		ConstructRenderable();
		ConstructPhysics();

		// Ensure at least one tick is performed.
		TickAnimation();
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
		AnimationTimeAccumulator += DeltaTime;

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();
		RenderData.Transform = WorldTransform;
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
	PrintColumn( Matrix[0] );
	PrintColumn( Matrix[1] );
	PrintColumn( Matrix[2] );
	PrintColumn( Matrix[3] );
}

void CMeshEntity::TickAnimation()
{
	if( !Renderable->GetRenderData().ShouldRender )
		return; // Mesh is invisible.

	// Profile( "Animation" );

	AnimationInstance.TickOffset = GetEntityID().ID;

	Animator::Update( AnimationInstance, AnimationTimeAccumulator, ForceAnimationTick );
	AnimationTimeAccumulator = 0.0; // Reset the accumulator.

	if( ForceAnimationTick )
	{
		ForceAnimationTick = false;
	}

	if( AnimationInstance.Bones.empty() )
		return;

	// Update the world bounds based on the bone locations.
	const auto& TransformReadOnly = Transform;
	WorldBounds = AnimationInstance.CalculateBounds( TransformReadOnly );

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
			const auto& TransformReadOnly = Transform;
			const auto DistanceSquared = TransformReadOnly.GetPosition().DistanceSquared( Camera->GetCameraPosition() );
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

	delete Renderable;

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

std::string GetMatrixLayout2( const Matrix4D& Matrix )
{
	const std::string BoneMatrix =
		std::to_string( Matrix[0][0] )
		+ " " + std::to_string( Matrix[0][1] )
		+ " " + std::to_string( Matrix[0][2] )
		+ " " + std::to_string( Matrix[0][3] )
		+ "\n" + std::to_string( Matrix[1][0] )
		+ " " + std::to_string( Matrix[1][1] )
		+ " " + std::to_string( Matrix[1][2] )
		+ " " + std::to_string( Matrix[1][3] )
		+ "\n" + std::to_string( Matrix[2][0] )
		+ " " + std::to_string( Matrix[2][1] )
		+ " " + std::to_string( Matrix[2][2] )
		+ " " + std::to_string( Matrix[2][3] )
		+ "\n" + std::to_string( Matrix[3][0] )
		+ " " + std::to_string( Matrix[3][1] )
		+ " " + std::to_string( Matrix[3][2] )
		+ " " + std::to_string( Matrix[3][3] )
		;

	return BoneMatrix;
}

void CMeshEntity::Debug()
{
	CPointEntity::Debug();

	const auto Position = WorldTransform.GetPosition();
	const auto ForwardVector = WorldTransform.Rotate( Vector3D( 0.0f, 1.0f, 0.0f ) ).Normalized();
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

		if( DisplaySkeleton )
		{
			AnimationInstance.Debug( GetTransform() );
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

	if( DisplayNormals && Mesh )
	{
		const auto& BufferData = Mesh->GetVertexBufferData();
		const auto& VertexData = Mesh->GetVertexData();

		static bool LogVertPos = true;
		Vector3D Normal, Tangent, Binormal;
		for( size_t Index = 0; Index < BufferData.VertexCount; Index++ )
		{
			const auto& Vertex = VertexData.Vertices[Index];

			ByteToVector( Vertex.Normal, Normal );
			ByteToVector( Vertex.Tangent, Tangent );

			const auto& TransformReadOnly = Transform;
			Matrix4D BoneTransform = TransformReadOnly.GetTransformationMatrix();
			if( !AnimationInstance.Bones.empty() )
			{
				BoneTransform = Matrix4D( 0.0f );

				// Compose the bone transformations.
				if( Vertex.Bone[0] > -1 )
				{
					BoneTransform += AnimationInstance.Bones[Vertex.Bone[0]].BoneTransform * Vertex.Weight[0];
				}

				if( Vertex.Bone[1] > -1 )
				{
					BoneTransform += AnimationInstance.Bones[Vertex.Bone[1]].BoneTransform * Vertex.Weight[1];
				}

				if( Vertex.Bone[2] > -1 )
				{
					BoneTransform += AnimationInstance.Bones[Vertex.Bone[2]].BoneTransform * Vertex.Weight[2];
				}

				if( Vertex.Bone[3] > -1 )
				{
					BoneTransform += AnimationInstance.Bones[Vertex.Bone[3]].BoneTransform * Vertex.Weight[3];
				}

				// Transform to the model location.
				BoneTransform = TransformReadOnly.GetTransformationMatrix() * BoneTransform;
			}

			Vector3D Position = BoneTransform.Transform( Vertex.Position );
			Normal = BoneTransform.Rotate( Normal ).Normalized();
			Tangent = BoneTransform.Rotate( Tangent ).Normalized();
			Binormal = Normal.Cross( Tangent ).Normalized();

			if( LogVertPos )
			{
				std::string Log = "Index: " + std::to_string( Index );
				Log += " Position: " + std::to_string( Position.X ) + " " + std::to_string( Position.Y ) + " " + std::to_string( Position.Z );
				Log += " Normal: " + std::to_string( Normal.X ) + " " + std::to_string( Normal.Y ) + " " + std::to_string( Normal.Z );

				if( Math::Equal( Normal, Vector3D::Zero ) )
				{
					Log += "\n";
					Log += "ByteNormal: " + std::to_string( Vertex.Normal[0] ) + " " + std::to_string( Vertex.Normal[1] ) + " " + std::to_string( Vertex.Normal[2] );
					Log += "\n";

					Log += GetMatrixLayout2( AnimationInstance.Bones[Vertex.Bone[0]].BoneTransform );
					Log += "\n";

					Log += GetMatrixLayout2( AnimationInstance.Bones[Vertex.Bone[1]].BoneTransform );
					Log += "\n";

					Log += GetMatrixLayout2( AnimationInstance.Bones[Vertex.Bone[2]].BoneTransform );
					Log += "\n";

					Log += GetMatrixLayout2( AnimationInstance.Bones[Vertex.Bone[3]].BoneTransform );
					Log += "\n";

					Log += "\nWeights " + std::to_string( Vertex.Weight[0] ) + " " + std::to_string( Vertex.Weight[1] ) + " " + std::to_string( Vertex.Weight[2] ) + " " + std::to_string( Vertex.Weight[3] );
				}

				Log::Event( "%s\n", Log.c_str() );
			}

			// Scale down the vectors to make them smaller on screen.
			Normal *= 0.025f;
			Tangent *= 0.025f;
			Binormal *= 0.025f;

			UI::AddLine( Position, Position + Normal, Color::Blue );
			UI::AddLine( Position, Position + Tangent, Color::Red );
			UI::AddLine( Position, Position + Binormal, Color::Green );
		}

		LogVertPos = false;
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
		else if( Property->Key == "damping" )
		{
			Extract( Property->Value, CollisionDamping );
		}
		else if( Property->Key == "friction" )
		{
			Extract( Property->Value, CollisionFriction );
		}
		else if( Property->Key == "restitution" )
		{
			Extract( Property->Value, CollisionRestitution );
		}
		else if( Property->Key == "drag" )
		{
			Extract( Property->Value, CollisionDrag );
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
			// AnimationInstance.CurrentAnimation = Property->Value;
			SetAnimation( Property->Value );
		}
		else if( Property->Key == "playrate" && Property->Value.length() > 0 )
		{
			AnimationInstance.PlayRate = ParseFloat( Property->Value.c_str() );
		}
		else if( Property->Key == "loop" && Property->Value.length() > 0 )
		{
			// TODO: LoopAnimation was removed from the main animation instance.
			// Extract( Property->Value, AnimationInstance.LoopAnimation );
		}
		else if( Property->Key == "maximum_render_distance" && Property->Value.length() > 0 )
		{
			Extract( Property->Value, MaximumRenderDistance );
		}
		else if( Property->Key == "material" && Property->Value.length() > 0 )
		{
			MaterialName = Property->Value;
		}
	}

	if( TextureNames.empty() )
	{
		TextureNames.emplace_back( "error" );
	}

	// Reload();
}

void CMeshEntity::Reload()
{
	CAssets& Assets = CAssets::Get();

	CMesh* TargetCollisionMesh = Assets.Meshes.Find( CollisionMeshName );
	if( TargetCollisionMesh )
	{
		CollisionMesh = TargetCollisionMesh;
	}

	CMesh* TargetMesh = Assets.Meshes.Find( MeshName );

	// Check if we have a material.
	Material = Assets.FindAsset<MaterialAsset>( MaterialName );
	if( Material )
	{
		Spawn( TargetMesh, nullptr, nullptr, Transform );
		return;
	}

	CShader* TargetShader = Assets.Shaders.Find( ShaderName );

	if( !TargetShader )
	{
		// Try to use the default shader if the specified shader can't be found.
		TargetShader = Assets.Shaders.Find( "default" );
	}

	Spawn( TargetMesh, TargetShader, nullptr, Transform );

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
	DataString::Decode( Data, MaterialName );

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

	Data >> CollisionDamping;
	Data >> CollisionFriction;
	Data >> CollisionRestitution;
	Data >> CollisionDrag;

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
	DataString::Encode( Data, MaterialName );
	
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

	Data << CollisionDamping;
	Data << CollisionFriction;
	Data << CollisionRestitution;
	Data << CollisionDrag;

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
		WorldBounds = Math::AABB( Mesh->GetBounds(), WorldTransform );
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

int32_t CMeshEntity::GetBoneIndex( const std::string& Name ) const
{
	return AnimationInstance.GetBoneIndex( Name );
}

Matrix4D CMeshEntity::GetBoneTransform( const int32_t Handle, const Space Space ) const
{
	if( Space == World )
	{
		// Transform the bone into world space.
		return Transform.GetTransformationMatrix() * AnimationInstance.GetBoneTransform( Handle );
	}

	return AnimationInstance.GetBoneTransform( Handle );
}

Vector3D CMeshEntity::GetRootMotion() const
{
	return Transform.GetRotationMatrix().Transform( AnimationInstance.RootMotion );
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

	// Configure the render data.
	FRenderDataInstanced& RenderData = Renderable->GetRenderData();
	RenderData.Transform = WorldTransform;
	RenderData.Color = Color;
	RenderData.WorldBounds = WorldBounds;

	// Set up animation data.
	const auto& Set = Mesh->GetAnimationSet();
	const auto& Skeleton = Set.Skeleton;
	if( !Skeleton.Bones.empty() && !Skeleton.Animations.empty() && GetAnimation().empty() )
	{
		const auto FirstAnimation = ( *Skeleton.Animations.begin() ).first;
		SetAnimation( FirstAnimation, true );
	}

	AnimationInstance.Mesh = Mesh;
	Renderable->SetMesh( Mesh );

	// Check if we have a material asset set.
	if( Material )
	{
		// Apply the material.
		Material->Material.Apply( Renderable );

		// Ignore further shading and texturing configuration steps.
		return;
	}

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

	// Create the body and set the collision type.
	PhysicsBody = new CBody();
	PhysicsBody->Type = CollisionType;

	// Adjust the defaults for non-spherical objects.
	if( CollisionType != BodyType::Sphere )
	{
		PhysicsBody->Restitution = 0.001f;
		PhysicsBody->Friction = 0.5f;
	}

	if( CollisionDamping > 0.0f )
	{
		PhysicsBody->Damping = CollisionDamping;
	}

	if( CollisionFriction > 0.0f )
	{
		PhysicsBody->Friction = CollisionFriction;
	}

	if( CollisionRestitution > 0.0f )
	{
		PhysicsBody->Restitution = CollisionRestitution;
	}

	if( CollisionDrag > 0.0f )
	{
		PhysicsBody->DragCoefficient = CollisionDrag;
	}

	PhysicsBody->Owner = this;
	PhysicsBody->Static = Static;
	PhysicsBody->Stationary = Stationary;
	PhysicsBody->Block = true;
	PhysicsBody->LocalBounds = Mesh->GetBounds();
	PhysicsBody->Construct( Physics );

	if( Material )
	{
		// Assign the material's physical surface by default.
		PhysicsBody->Surface = Material->Material.Surface;
	}
}

void CMeshEntity::QueueRenderable( CRenderable* Renderable )
{
	CRenderer& Renderer = CWindow::Get().GetRenderer();
	Renderer.QueueRenderable( Renderable );
}
