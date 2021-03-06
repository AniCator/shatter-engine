// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "MeshEntity.h"

#include <Game/Game.h>

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

static CEntityFactory<CMeshEntity> Factory( "mesh" );

CMeshEntity::CMeshEntity()
{
	Mesh = nullptr;
	CollisionMesh = nullptr;
	Shader = nullptr;
	Textures.reserve( 8 );
	Renderable = nullptr;
	PhysicsBody = nullptr;
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
				if( !PhysicsBody )
				{
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

					if( PhysicsBody )
					{
						PhysicsBody->Construct( Physics );
					}
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

		QueueRenderable( Renderable );
	}
}

const float Duration = 4.8f;
void TransformBones( const float& Time, const Skeleton& Skeleton, const Animation& Animation, const Bone* Bone, std::vector<::Bone>& Result )
{
	if( !Bone )
	{
		return;
	}

	Matrix4D TranslationMatrix, RotationMatrix, ScaleMatrix;
	for( auto& Key : Animation.PositionKeys )
	{
		if( Key.BoneIndex != Bone->Index )
			continue;

		float RelativeTime = ( Key.Time / Animation.Duration ) * Duration;
		if( RelativeTime < Time )
		{
			auto Position = glm::mat4();
			glm::translate( Position, glm::vec3( Key.Value.X, Key.Value.Y, Key.Value.Z ) );
			TranslationMatrix = Math::FromGLM( Position );
		}
	}

	for( auto& Key : Animation.RotationKeys )
	{
		if( Key.BoneIndex != Bone->Index )
			continue;

		float RelativeTime = ( Key.Time / Animation.Duration ) * Duration;
		if( RelativeTime < Time )
		{
			auto Quaternion = glm::quat( Key.Value.X, Key.Value.Y, Key.Value.Z, Key.Value.W );
			RotationMatrix = Math::FromGLM( glm::toMat4( Quaternion ) );
		}
	}

	for( auto& Key : Animation.ScalingKeys )
	{
		if( Key.BoneIndex != Bone->Index )
			continue;

		float RelativeTime = ( Key.Time / Animation.Duration ) * Duration;
		if( RelativeTime < Time )
		{
			auto Scale = glm::mat4();
			glm::scale( Scale, glm::vec3( Key.Value.X, Key.Value.Y, Key.Value.Z ) );
			ScaleMatrix = Math::FromGLM( Scale );
		}
	}

	auto BoneMatrix = TranslationMatrix * RotationMatrix * ScaleMatrix;
	if( Bone->ParentIndex > -1 )
	{
		BoneMatrix = Skeleton.Bones[Bone->ParentIndex].Matrix * BoneMatrix;
	}

	Result[Bone->Index].Matrix = Skeleton.GlobalMatrixInverse * Skeleton.GlobalMatrix * BoneMatrix;

	for( size_t ChildIndex = 0; ChildIndex < Bone->Children.size(); ChildIndex++ )
	{
		if( Bone->Children[ChildIndex] > -1 && Bone->Children[ChildIndex] < Result.size() )
		{
			TransformBones( Time, Skeleton, Animation, &Skeleton.Bones[Bone->Children[ChildIndex]], Result );
		}
	}
};

void CMeshEntity::TickAnimation()
{
	if( !Mesh )
	{
		return;
	}

	auto& Skeleton = Mesh->GetSkeleton();
	if( Skeleton.Bones.empty() )
	{
		return;
	}

	if( Skeleton.RootIndex < 0 )
	{
		return;
	}

	Profile( "Animation" );

	CurrentAnimation = "BoyRig|Idle";
	const auto& Iterator = Skeleton.Animations.find( CurrentAnimation );
	if( Iterator == Skeleton.Animations.end() )
	{
		return;
	}

	// UI::AddText( Transform.GetPosition(), std::to_string( ModulatedTime ).c_str() );

	auto& Pair = *Iterator;
	auto& Animation = Pair.second;

	const float Time = static_cast<float>( GameLayersInstance->GetCurrentTime() );
	const float ModulatedTime = fmod( Time, Duration );

	Bones.resize( Skeleton.Bones.size() );
	TransformBones( ModulatedTime, Skeleton, Animation, &Skeleton.Bones[Skeleton.RootIndex], Bones );

	const std::string BoneLocationNamePrefix = "Bones[";

	auto WorldTransform = GetTransform();
	for( size_t MatrixIndex = 0; MatrixIndex < Bones.size(); MatrixIndex++ )
	{
		auto& Matrix = Bones[MatrixIndex].Matrix;
		glm::mat4 GLMMatrix = Math::ToGLM( Matrix );
		auto WorldMatrix = Math::FromGLM( glm::inverse( GLMMatrix ) );
		Vector3D PointA = WorldTransform.Transform( WorldMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) ) );
		Vector3D PointB = WorldTransform.Transform( WorldMatrix.Transform( Vector3D( 0.0f, 0.0f, -0.5f ) ) );
		Vector3D PointC = WorldTransform.Transform( WorldMatrix.Transform( Vector3D( 0.0f, 0.0f, -1.0f ) ) );
		UI::AddCircle( PointA, 3.0f, ::Color( 0, 0, 255 ) );
		UI::AddLine( PointA, PointB, ::Color( 0, 0, 255 ) );
		UI::AddLine( PointB, PointC, ::Color( 255, 0, 0 ) );
		UI::AddCircle( PointC, 3.0f, ::Color( 255, 0, 0 ) );
		UI::AddText( PointC, Skeleton.MatrixNames[MatrixIndex].c_str() );

		// Bones[Bones[MatrixIndex].ParentIndex]

		// UI::AddText( PointC + Vector3D( 0.0f, 0.0f, -0.1f ), std::to_string( ChosenTime ).c_str() );

		if( Renderable )
		{
			std::string BoneLocationName = BoneLocationNamePrefix + std::to_string( MatrixIndex ) + "]";
			Renderable->SetUniform( BoneLocationName, Matrix );
		}
	}
}

void CMeshEntity::Frame()
{
	if( Renderable && IsVisible() )
	{
		TickAnimation();
	}
}

void CMeshEntity::Destroy()
{
	if( PhysicsBody )
	{
		PhysicsBody->Destroy( GetWorld()->GetPhysics() );
		delete PhysicsBody;
		PhysicsBody = nullptr;
	}

	CPointEntity::Destroy();
}

void CMeshEntity::Debug()
{
	CPointEntity::Debug();

	auto Camera = GetWorld()->GetActiveCamera();
	if( Camera )
	{
		auto Frustum = Camera->GetFrustum();
		auto Position = Transform.GetPosition();

		const bool Contains = Frustum.Contains( Position );
		if( Contains )
		{
			// UI::AddText( Position + Vector3D( 1.0f, 0.0f, 0.0f ), "Visible" );
		}
		else
		{
			// UI::AddText( Position + Vector3D( 1.0f, 0.0f, 0.0f ), "Invisible" );
		}
	}

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

	std::string CollisionTypeString;
	DataString::Decode( Data, CollisionTypeString );
	CollisionType = ToBodyType( CollisionTypeString );
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
	DataString::Encode( Data, FromBodyType( CollisionType ) );
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

FBounds CMeshEntity::GetWorldBounds() const
{
	return WorldBounds;
}

CBody* CMeshEntity::GetBody() const
{
	return PhysicsBody;
}

void CMeshEntity::QueueRenderable( CRenderable* Renderable )
{
	CRenderer& Renderer = CWindow::Get().GetRenderer();
	Renderer.QueueRenderable( Renderable );
}
