// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Animation/Animator.h>
#include <Engine/Physics/Body/Shared.h>
#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math.h>

#include <Engine/Animation/Skeleton.h>

class CMesh;
class CShader;
class CTexture;
class CRenderable;
class CBody;
class MaterialAsset;

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

	const Animator::Instance& GetAnimationInstance() const
	{
		return AnimationInstance;
	}

	bool IsAnimationFinished() const;

	float GetPlayRate() const;
	void SetPlayRate( const float& PlayRate );

	float GetAnimationTime() const;
	void SetAnimationTime( const float& Value );

	int32_t GetBoneIndex( const std::string& Name ) const;

	enum Space
	{
		Local,
		World
	};
	Matrix4D GetBoneTransform( const int32_t Handle, const Space Space = World ) const;

	Vector3D GetRootMotion() const;

	void SetPosition( const Vector3D& Position, const bool& Teleport = true );
	void SetOrientation( const Vector3D& Orientation );
	void SetSize( const Vector3D& Size );

	CMesh* Mesh;
	CMesh* CollisionMesh;
	CShader* Shader;
	std::vector<CTexture*> Textures;
	MaterialAsset* Material = nullptr;
	CRenderable* Renderable;

	Vector4D Color = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f );

	std::string MeshName;
	std::string CollisionMeshName;
	BodyType CollisionType = BodyType::AABB;
	bool ShouldProject = false;
	std::string ShaderName;
	std::vector<std::string> TextureNames;
	std::string MaterialName;

	bool Contact;
	bool Visible;

	// Set to true when the sequencer is making use of this entity.
	bool UsedBySequence = false;

	// Distance from the camera at which the object should be culled, infinite when negative.
	float MaximumRenderDistance = -1.0f;

	// When true, the LightOrigin parameter is used instead of the transform origin for fetching lighting information.
	bool UseLightOrigin = false;

	// Location used for fetching light data when UseLightOrigin is true.
	Vector3D LightOrigin = Vector3D::Zero;
protected:
	void ConstructRenderable();
	void ConstructPhysics();

	static void QueueRenderable( CRenderable* Renderable );
	BoundingBox WorldBounds;

	bool Collision;
	bool Static;
	bool Stationary;
	CBody* PhysicsBody;

	Animator::Instance AnimationInstance;

	// Ensures an animation tick will be performed on the next tick.
	bool ForceAnimationTick = false;

	// Should TickAnimation be called during the next game frame?
	bool WantsAnimationUpdate = false;
	double AnimationTimeAccumulator = 0.0;
};
