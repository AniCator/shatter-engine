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

	virtual FBounds GetWorldBounds() const;

public:
	CMesh* Mesh;
	CMesh* CollisionMesh;
	CShader* Shader;
	std::vector<CTexture*> Textures;
	CRenderable* Renderable;

	glm::vec4 Color;

	std::string MeshName;
	std::string CollisionMeshName;
	BodyType CollisionType = BodyType::TriangleMesh;
	std::string ShaderName;
	std::vector<std::string> TextureNames;

	bool Contact;
	bool Visible;

protected:
	FBounds WorldBounds;

	bool Collision;
	bool Static;
	bool Stationary;
	CBody* PhysicsComponent;

	// Name of the current animation.
	std::string CurrentAnimation;

	// Current bone transformations, updated by TickAnimation.
	std::vector<Bone> Bones;
};
