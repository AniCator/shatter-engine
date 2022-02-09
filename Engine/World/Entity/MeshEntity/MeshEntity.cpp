// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "MeshEntity.h"

#include <Game/Game.h>

#include <Engine/Animation/Animator.h>
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
	Stationary = false;
	Contact = false;
	Collision = true;
	Visible = true;

	Color = Vector4D( 1.0f, 1.0f, 1.0f, 1.0f );

	Inputs["PlayAnimation"] = [&] ( CEntity* Origin )
	{
		SetAnimation( CurrentAnimation );

		return true;
	};

	Inputs["LoopAnimation"] = [&] ( CEntity* Origin )
	{
		SetAnimation( CurrentAnimation, true );

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

		TickAnimation();

		FRenderDataInstanced& RenderData = Renderable->GetRenderData();
		RenderData.Transform = Transform;
		RenderData.Color = Color;
		RenderData.WorldBounds = WorldBounds;

		if( RenderData.LightIndex.Index[0] == -1 || !Static || true )
		{
			RenderData.LightIndex = LightEntity::Fetch( RenderData.Transform.GetPosition() );
		}

		QueueRenderable( Renderable );
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

size_t GetNextKey( const size_t& CurrentIndex, const std::vector<Key>& Keys )
{
	const auto& CurrentKey = Keys[CurrentIndex];
	for( size_t KeyIndex = CurrentIndex; KeyIndex < Keys.size(); KeyIndex++ )
	{
		if( CurrentKey.BoneIndex != Keys[KeyIndex].BoneIndex )
			continue;

		if( CurrentIndex == KeyIndex )
			continue;

		return KeyIndex;
	}

	return CurrentIndex;
}

void TransformBones( const float& Time, const Skeleton& Skeleton, const std::vector<AnimationBlendEntry>& Animations, const Bone* Parent, const Bone* Bone, std::vector<::Bone>& Result )
{
	if( !Bone )
	{
		return;
	}

	Animator::CompoundKey Blend;
	for( auto& Array : Animations )
	{
		const auto Key = Animator::Get( Array.Animation, fmod( Time, Array.Animation.Duration ), Bone->Index );
		Blend = Animator::Blend( Blend, Key, Array.Weight );
	}
	
	const auto Matrices = Animator::Get( Blend );

	Result[Bone->Index] = *Bone;

	// Local bone data
	const auto& ModelToBone = Bone->ModelToBone;
	const auto& BoneToModel = Bone->BoneToModel;
	const auto& ModelMatrix = Bone->ModelMatrix;
	const auto& InverseModelMatrix = Bone->InverseModelMatrix;
	
	// Concatenate all the keyframe transformations.
	const auto LocalTransform = Matrices.Translation * Matrices.Rotation * Matrices.Scale;
	
	Result[Bone->Index].LocalTransform = LocalTransform;

	// Look up the parent matrix.
	if( Parent )
	{
		Result[Bone->Index].GlobalTransform = Parent->GlobalTransform * LocalTransform;
	}
	else
	{
		// Set the global transform to just the local transform if no parent is found.
		Result[Bone->Index].GlobalTransform = LocalTransform;
	}

	Result[Bone->Index].BoneTransform = Result[Bone->Index].GlobalTransform * ModelToBone;

	for( const int& ChildIndex : Bone->Children )
	{
		if( ChildIndex > -1 && ChildIndex < Result.size() )
		{
			TransformBones( Time, Skeleton, Animations, &Result[Bone->Index], &Skeleton.Bones[ChildIndex], Result );
		}
	}
};

const std::string BoneLocationNamePrefix = "Bones[";
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

	Animation Animation;
	if( !Set.Lookup( CurrentAnimation, Animation ) )
	{
		CurrentAnimation = "";
		return;
	}

	const float DeltaTime = static_cast<float>( GameLayersInstance->GetDeltaTime() ) * PlayRate;
	const auto NewTime = AnimationTime + DeltaTime;
	if( !LoopAnimation && NewTime > Animation.Duration )
	{
		AnimationFinished = true;
		return;
	}

	// Update the animation time before throttling
	AnimationTime = fmod( NewTime, Animation.Duration );

	if( AnimationTickRate == 0 && !ForceAnimationTick )
		return; // Animation disabled.

	if( !Renderable->GetRenderData().ShouldRender )
		return; // Mesh is invisible.

	// Perform a reduced amount of animation updates when the tick rate is staggered above 1 tick.
	if( AnimationTickRate > 1 && !ForceAnimationTick )
	{
		// Stagger using the entity ID.
		const auto TickDelta = ( GetWorld()->GetTicks() + GetEntityID().ID ) % AnimationTickRate;
		if( TickDelta != 0 )
		{
			// Skip this tick.
			return;
		}
	}

	if( ForceAnimationTick )
	{
		ForceAnimationTick = false;
	}

	Profile( "Animation" );

	/*if( IsDebugEnabled() )
	{
		const auto TimeText = "Animation time: " + std::to_string( AnimationTime ) + "\nAnimation duration: " + std::to_string( Animation.Duration );
		UI::AddText( Transform.GetPosition() + Vector3D( 0.5f, 0.0f, -0.1f ), TimeText.c_str() );
	}*/

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

	// Re-weigh the blend stack.
	/*float TotalWeight = 0.0f;
	float Sum = 0.0f;
	for( const auto& Entry : BlendStack )
	{
		TotalWeight += Entry.Weight;
		Sum += 1.0f;
	}

	const auto Ratio = Sum / TotalWeight;
	for( auto& Entry : BlendStack )
	{
		Entry.Weight *= Ratio;
	}*/

	// Ensure the first weight is always present.
	if( !BlendStack.empty() )
	{
		BlendStack.front().Weight = 1.0f;
	}

	// Transform all of the bones in the skeletal hierarchy.
	for( const auto* Bone : RootBones )
	{
		TransformBones( AnimationTime, Skeleton, BlendStack, nullptr, Bone, Bones );
	}

	// Set global transform
	// Bones[Skeleton.RootIndex].Matrix = WorldTransform.GetTransformationMatrix() * Skeleton.Bones[Skeleton.RootIndex].Matrix;

	static int DebugIndex = 0;
	DebugIndex++;
	DebugIndex = DebugIndex % Bones.size();

	auto WorldTransform = GetTransform();
	// WorldTransform = FTransform();
	static auto NewBoundVertices = std::vector<Vector3D>();
	NewBoundVertices.clear();
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

		const auto Current = Matrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
		Vector3D PointTarget = WorldTransform.Transform( Current );
		NewBoundVertices.emplace_back( PointTarget );
		
		if( IsDebugEnabled() )
		{
			Vector3D RandomJitter = Vector3D( Math::Random(), Math::Random(), Math::Random() ) * 0.001f;
			const auto Parent = ParentMatrix.Transform( Vector3D( 0.0f, 0.0f, 0.0f ) );
			Vector3D PointSource = WorldTransform.Transform( Parent );

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
	}

	WorldBounds = Math::AABB( NewBoundVertices.data(), NewBoundVertices.size() );
	FRenderDataInstanced& RenderData = Renderable->GetRenderData();
	RenderData.WorldBounds = WorldBounds;

	SubmitAnimation();
}

void CMeshEntity::Frame()
{
	// 
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

	if( Renderable && false )
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
		else if( Property->Key == "loop" && Property->Value.length() > 0 )
		{
			Extract( Property->Value, LoopAnimation );
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
	const auto SameAnimation = CurrentAnimation == Name;
	CurrentAnimation = Name;
	LoopAnimation = Loop;
	AnimationFinished = false;
	AnimationTime = 0.0f;

	ForceAnimationTick = true;

	if( !Mesh && !SameAnimation )
		return;

	AnimationBlendEntry Entry;
	Entry.Weight = 1.0f;

	const auto& Set = Mesh->GetAnimationSet();
	if( !Set.Lookup( CurrentAnimation, Entry.Animation ) )
		return;

	BlendStack.clear();
	BlendStack.emplace_back( Entry );
}

const std::string& CMeshEntity::GetAnimation() const
{
	return CurrentAnimation;
}

bool CMeshEntity::HasAnimation( const std::string& Name ) const
{
	if( !Mesh )
		return false;

	static auto Dummy = Animation();
	const auto& Set = Mesh->GetAnimationSet();
	if( Set.Lookup( Name, Dummy ) )
		return true;

	return false;
}

bool CMeshEntity::IsAnimationFinished() const
{
	if( LoopAnimation )
		return true;

	return AnimationFinished;
}

float CMeshEntity::GetPlayRate() const
{
	return PlayRate;
}

void CMeshEntity::SetPlayRate( const float& PlayRate )
{
	this->PlayRate = PlayRate;
}

float CMeshEntity::GetAnimationTime() const
{
	return AnimationTime;
}

void CMeshEntity::SetAnimationTime( const float& Value )
{
	AnimationTime = Value;
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

void CMeshEntity::SubmitAnimation()
{
	if( !Renderable )
		return;

	for( size_t MatrixIndex = 0; MatrixIndex < Bones.size(); MatrixIndex++ )
	{
		const std::string BoneLocationName = BoneLocationNamePrefix + std::to_string( MatrixIndex ) + "]";
		Renderable->SetUniform( BoneLocationName, Bones[MatrixIndex].BoneTransform );
	}

	if( !Bones.empty() )
	{
		Renderable->HasSkeleton = true;
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
		SetAnimation( FirstAnimation, LoopAnimation );
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
