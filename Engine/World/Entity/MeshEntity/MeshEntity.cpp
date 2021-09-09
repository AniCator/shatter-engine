// Copyright © 2017, Christiaan Bakker, All rights reserved.
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

#include <Engine/World/Entity/LightEntity/LightEntity.h>

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

	Color = Vector4D( 1.0f, 1.0f, 1.0f, 1.0f );
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
		RenderData.WorldBounds = WorldBounds;

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

		const auto& Set = Mesh->GetAnimationSet();
		const auto& Skeleton = Set.Skeleton;
		if( !Skeleton.Bones.empty() && !Skeleton.Animations.empty() )
		{
			const auto FirstAnimation = ( *Skeleton.Animations.begin() ).first;
			SetAnimation( FirstAnimation );
		}
	}
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

	if( IsVisible() && Renderable )
	{
		if( ShouldUpdateTransform )
		{
			GetTransform();
		}

		TickAnimation();

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();
		RenderData.Transform = Transform;
		RenderData.Color = Color;
		RenderData.WorldBounds = WorldBounds;

		if( RenderData.LightIndex.Index[0] == -1 || !Static )
		{
			RenderData.LightIndex = LightEntity::Fetch( RenderData.Transform.GetPosition() );
		}

		QueueRenderable( Renderable );
	}
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

size_t NearestKey( const float& Time, const float& Duration, const Bone* Bone, const std::vector<Key>& Keys )
{
	size_t Output = 0;
	for( size_t KeyIndex = 0; KeyIndex < Keys.size(); KeyIndex++ )
	{
		if( Bone->Index != Keys[KeyIndex].BoneIndex )
			continue;
		
		const float RelativeTime = ( Keys[KeyIndex].Time / Duration ) * Duration;
		if( RelativeTime < Time )
		{
			Output = KeyIndex;
		}
	}

	return Output;
}

void TransformBones( const CMeshEntity* Entity, const float& Time, const Skeleton& Skeleton, const Animation& Animation, const Bone* Parent, const Bone* Bone, std::vector<::Bone>& Result )
{
	if( !Bone )
	{
		return;
	}

	Matrix4D TranslationMatrix = Matrix4D();
	Matrix4D RotationMatrix = Matrix4D();
	Matrix4D ScaleMatrix = Matrix4D();

	const auto PositionKeyIndex = NearestKey( Time, Animation.Duration, Bone, Animation.PositionKeys );
	const auto PositionKey = Animation.PositionKeys[PositionKeyIndex];
	auto Position = glm::mat4();
	Position = glm::translate( Position, glm::vec3( PositionKey.Value.X, PositionKey.Value.Y, PositionKey.Value.Z ) );
	TranslationMatrix = Math::FromGLM( Position );

	if( Entity && Entity->IsDebugEnabled() )
	{
		UI::AddCircle( TranslationMatrix.Transform( Vector3D::Zero ), 3.0f, Color( 255, 0, 0 ) );
	}

	const auto RotationKeyIndex = NearestKey( Time, Animation.Duration, Bone, Animation.RotationKeys );
	const auto RotationKey = Animation.RotationKeys[RotationKeyIndex];
	auto Quaternion = glm::quat( RotationKey.Value.X, RotationKey.Value.Y, RotationKey.Value.Z, RotationKey.Value.W );

	// auto Euler = glm::eulerAngles( Quaternion );
	// Euler.x *= -1.0f;
	// Quaternion = glm::quat( Euler );
	
	/*if( Bone->ParentIndex < 0 || true )
	{
		auto Correction = glm::quat( glm::vec3( Math::ToRadians( -90.0f ), 0.0f, Math::ToRadians( 180.0f ) ) );
		Quaternion = Quaternion * Correction;
	}*/
	
	RotationMatrix = Math::FromGLM( glm::toMat4( Quaternion ) );

	if( Entity && Entity->IsDebugEnabled() )
	{
		UI::AddCircle( RotationMatrix.Transform( Vector3D( 0.0f, 1.0f, 0.0f ) ), 3.0f, Color( 0, 255, 0 ) );
		UI::AddLine( RotationMatrix.Transform( Vector3D( 0.0f, 1.0f, 0.0f ) ), Vector3D::Zero );
	}

	const auto ScalingKeyIndex = NearestKey( Time, Animation.Duration, Bone, Animation.ScalingKeys );
	const auto ScalingKey = Animation.ScalingKeys[ScalingKeyIndex];
	auto Scale = glm::mat4();
	Scale = glm::scale( Scale, glm::vec3( ScalingKey.Value.X, ScalingKey.Value.Y, ScalingKey.Value.Z ) );
	ScaleMatrix = Math::FromGLM( Scale );

	if( Entity && Entity->IsDebugEnabled() )
	{
		UI::AddCircle( ScaleMatrix.Transform( Vector3D( 1.0f, 1.0f, 1.0f ) ), 3.0f, Color( 0, 0, 255 ) );
	}

	Result[Bone->Index] = *Bone;

	// Local bone data
	const auto& ModelToBone = Bone->ModelToBone;
	const auto& BoneToModel = Bone->BoneToModel;
	const auto& ModelMatrix = Bone->ModelMatrix;
	const auto& InverseModelMatrix = Bone->InverseModelMatrix;
	
	// Concatenate all the keyframe transformations.
	auto LocalTransform = TranslationMatrix * RotationMatrix * ScaleMatrix;
	
	Result[Bone->Index].LocalTransform = LocalTransform;

	// Create the global transform.
	Result[Bone->Index].GlobalTransform = LocalTransform;

	// Look up the parent matrix.
	if( Parent )
	{
		// TODO: It seems like the rotation is being applied twice.
		// TODO: See the SkeletonTest model for reference, its tip Root.001 cube is rotate multiple times.
		Result[Bone->Index].GlobalTransform = Parent->GlobalTransform * LocalTransform;
	}

	Result[Bone->Index].GlobalTransform = Result[Bone->Index].GlobalTransform;

	Result[Bone->Index].BoneTransform = Result[Bone->Index].GlobalTransform * ModelToBone;

	for( const int& ChildIndex : Bone->Children )
	{
		if( ChildIndex > -1 && ChildIndex < Result.size() )
		{
			TransformBones( Entity, Time, Skeleton, Animation, &Result[Bone->Index], &Skeleton.Bones[ChildIndex], Result );
		}
	}
};

void CMeshEntity::TickAnimation()
{
	if( !Mesh )
	{
		return;
	}

	const auto& Set = Mesh->GetAnimationSet();
	const auto& Skeleton = Set.Skeleton;
	if( Skeleton.Bones.empty() )
	{
		return;
	}

	if( Skeleton.RootIndex < 0 )
	{
		return;
	}

	Profile( "Animation" );

	Animation Animation;
	if( !Set.Lookup( CurrentAnimation, Animation ) )
	{
		return;
	}

	const float DeltaTime = static_cast<float>( GameLayersInstance->GetDeltaTime() ) * PlayRate;
	AnimationTime = fmod( AnimationTime + DeltaTime, Animation.Duration );
	

	if( IsDebugEnabled() )
	{
		const auto TimeText = "Animation time: " + std::to_string( AnimationTime ) + "\nAnimation duration: " + std::to_string( Animation.Duration );
		UI::AddText( Transform.GetPosition() + Vector3D( 0.5f, 0.0f, -0.1f ), TimeText.c_str() );
	}

	Bones.clear();
	Bones.resize( Skeleton.Bones.size() );

	// Reset the matrices.
	for( auto& Bone : Bones )
	{
		Bone.BoneTransform = Matrix4D();
		Bone.GlobalTransform = Matrix4D();
	}

	// Collect the root bones since it's possible for multiple bones to be disconnected.
	std::vector<const Bone*> RootBones;
	for( const auto& Bone : Skeleton.Bones )
	{
		if( Bone.ParentIndex < 0 )
		{
			RootBones.emplace_back( &Bone );
		}
	}

	// Transform all of the bones in the skeletal hierarchy.
	for( const auto* Bone : RootBones )
	{
		TransformBones( this, AnimationTime, Skeleton, Animation, nullptr, Bone, Bones );
	}

	// Set global transform
	// Bones[Skeleton.RootIndex].Matrix = WorldTransform.GetTransformationMatrix() * Skeleton.Bones[Skeleton.RootIndex].Matrix;

	const std::string BoneLocationNamePrefix = "Bones[";

	static int DebugIndex = 0;
	DebugIndex++;
	DebugIndex = DebugIndex % Bones.size();

	auto WorldTransform = GetTransform();
	// WorldTransform = FTransform();
	for( size_t MatrixIndex = 0; MatrixIndex < Bones.size(); MatrixIndex++ )
	{
		auto Matrix = Bones[MatrixIndex].GlobalTransform;
		auto ParentMatrix = Matrix;
		if( Bones[MatrixIndex].ParentIndex > -1 )
		{
			ParentMatrix = Bones[Bones[MatrixIndex].ParentIndex].GlobalTransform;
		}

		// Matrix = Skeleton.GlobalMatrixInverse * Matrix;
		// ParentMatrix = Skeleton.GlobalMatrixInverse * ParentMatrix;
		
		if( ( MatrixIndex < 1 || true ) && IsDebugEnabled() )
		{
			Vector3D RandomJitter = Vector3D( Math::Random(), Math::Random(), Math::Random() ) * 0.001f;
			const auto Parent = ParentMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
			Vector3D PointSource = WorldTransform.Transform( Parent );
			
			const auto Current = Matrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );			
			Vector3D PointTarget = WorldTransform.Transform( Current );

			Vector3D PointCenter = PointSource + ( PointTarget - PointSource ) * 0.5f;

			const auto Bind = Bones[MatrixIndex].BoneToModel.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
			Vector3D PointBind = WorldTransform.Transform( Bind );

			auto ParentBindMatrix = Bones[MatrixIndex].BoneToModel;
			if( Bones[MatrixIndex].ParentIndex > -1 )
			{
				ParentBindMatrix = Bones[Bones[MatrixIndex].ParentIndex].BoneToModel;
			}

			const auto ParentBind = ParentBindMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
			Vector3D PointParentBind = WorldTransform.Transform( ParentBind );

			// FBounds AABB( ( Vector3D::One * -1.0f ), Vector3D::One );
			// AABB = Math::AABB( AABB, Matrix );
			// UI::AddAABB( AABB.Minimum, AABB.Maximum );
			
			UI::AddCircle( PointSource, 3.0f, ::Color( 0, 0, 255 ) );
			UI::AddLine( PointSource, PointCenter, ::Color( 0, 0, 255 ) );
			UI::AddLine( PointCenter, PointTarget, ::Color( 255, 0, 0 ) );
			UI::AddCircle( PointTarget, 3.0f, ::Color( 255, 0, 0 ) );
			UI::AddText( PointTarget, Skeleton.MatrixNames[MatrixIndex].c_str() );

			UI::AddCircle( PointParentBind, 3.0f, ::Color( 0, 255, 255 ) );
			UI::AddLine( PointParentBind, PointBind, ::Color( 255, 255, 0 ) );
			UI::AddCircle( PointBind, 3.0f, ::Color( 0, 255, 255 ) );
			// UI::AddText( PointBind, Skeleton.MatrixNames[MatrixIndex].c_str() );
			
			// 
			// int TextIndex = MatrixIndex * 1;
			// const std::string MatrixName = Skeleton.MatrixNames[MatrixIndex] + "[" + std::to_string( Skeleton.Bones[MatrixIndex].Index ) + "]";
			// UI::AddText( Vector2D( 30.0f, 30.0f + ( TextIndex + 0 ) * 20.0f ), MatrixName, Origin );
		}

		// Bones[Bones[MatrixIndex].ParentIndex]

		// UI::AddText( PointC + Vector3D( 0.0f, 0.0f, -0.1f ), std::to_string( ChosenTime ).c_str() );

		if( Renderable )
		{
			const std::string BoneLocationName = BoneLocationNamePrefix + std::to_string( MatrixIndex ) + "]";
			Renderable->SetUniform( BoneLocationName, Bones[MatrixIndex].BoneTransform );
		}
	}
}

void CMeshEntity::Frame()
{
	/*if( Renderable && IsVisible() )
	{
		TickAnimation();
	}*/
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

	if( Renderable )
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
			CurrentAnimation = Property->Value;
		}
		else if( Property->Key == "playrate" && Property->Value.length() > 0 )
		{
			PlayRate = ParseFloat( Property->Value.c_str() );
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
	CMesh* TargetMesh = Assets.FindMesh( MeshName );
	CShader* TargetShader = Assets.FindShader( ShaderName );

	if( !TargetShader )
	{
		// Try to use the default shader if the specified shader can't be found.
		TargetShader = Assets.FindShader( "default" );
	}

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

void CMeshEntity::SetAnimation( const std::string& Name )
{
	CurrentAnimation = Name;
}

const std::string& CMeshEntity::GetAnimation() const
{
	return CurrentAnimation;
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

void CMeshEntity::QueueRenderable( CRenderable* Renderable )
{
	CRenderer& Renderer = CWindow::Get().GetRenderer();
	Renderer.QueueRenderable( Renderable );
}
