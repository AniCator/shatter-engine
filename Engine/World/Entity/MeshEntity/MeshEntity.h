// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Physics/Body/Shared.h>
#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math.h>

#include <Engine/Animation/Skeleton.h>

class CMesh;
class CShader;
class CTexture;
class CRenderable;
class CBody;

struct AnimationBlendEntry
{
	AnimationBlendEntry() = default;
	AnimationBlendEntry( const Animation& Anim, const float& Weight )
	{
		this->Animation = Anim;
		this->Weight = Weight;
	}

	Animation Animation{};
	float Weight = 1.0f;
};

class CMeshEntity : public CPointEntity
{
public:
	CMeshEntity();
	CMeshEntity( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform );
	virtual ~CMeshEntity() override;

	virtual void Spawn( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform );

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void TickAnimation();
	virtual void Frame() override;
	virtual void Destroy() override;

	virtual void Debug() override;

	virtual void Load( const JSON::Vector& Objects ) override;
	virtual void Reload() override;

	virtual void Import( CData& Data ) override;
	virtual void Export( CData& Data ) override;

	virtual bool ShouldCollide() const;
	virtual void SetCollision( const bool Enable );
	
	virtual bool IsStatic() const;
	virtual bool IsStationary() const;

	virtual bool IsVisible() const;
	virtual void SetVisible( const bool& Visible );

	virtual BoundingBox GetWorldBounds() const;
	virtual CBody* GetBody() const;

	virtual const FTransform& GetTransform() override;

	void SetAnimation( const std::string& Name, const bool& Loop = false );
	const std::string& GetAnimation() const;
	bool HasAnimation( const std::string& Name ) const;

	bool IsAnimationFinished() const;

	float GetPlayRate() const;
	void SetPlayRate( const float& PlayRate );

	float GetAnimationTime() const;
	void SetAnimationTime( const float& Value );

	void SetPosition( const Vector3D& Position, const bool& Teleport = true );
	void SetOrientation( const Vector3D& Orientation );
	void SetSize( const Vector3D& Size );

public:
	CMesh* Mesh;
	CMesh* CollisionMesh;
	CShader* Shader;
	std::vector<CTexture*> Textures;
	CRenderable* Renderable;

	Vector4D Color = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );

	std::string MeshName;
	std::string CollisionMeshName;
	BodyType CollisionType = BodyType::AABB;
	bool ShouldProject = false;
	std::string ShaderName;
	std::vector<std::string> TextureNames;

	bool Contact;
	bool Visible;

	// Animation blending stack.
	std::vector<AnimationBlendEntry> BlendStack;

	// Distance from the camera at which the object should be culled, infinite when negative.
	float MaximumRenderDistance = -1.0f;

protected:
	void SubmitAnimation();

	void ConstructRenderable();
	void ConstructPhysics();

	static void QueueRenderable( CRenderable* Renderable );
	BoundingBox WorldBounds;

	bool Collision;
	bool Static;
	bool Stationary;
	CBody* PhysicsBody;

	// Name of the current animation.
	std::string CurrentAnimation;
	bool LoopAnimation = false;
	bool AnimationFinished = true;

	// Current bone transformations, updated by TickAnimation.
	std::vector<Bone> Bones;

	float AnimationTime = 0.0f;
	float PlayRate = 1.0f;

	// Determines how often animations should tick. (zero means, never)
	uint32_t AnimationTickRate = 1;

	// Ensures an animation tick will be performed on the next tick.
	bool ForceAnimationTick = false;
};
