// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/Entity.h>
#include <Engine/Utility/Math.h>

class CMesh;
class CShader;
class CTexture;
class CRenderable;

class CMeshEntity : public CEntity
{
public:
	CMeshEntity();
	CMeshEntity( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform );
	virtual ~CMeshEntity() override;

	void Spawn( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform );

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;

	virtual const FTransform& GetTransform() const { return Transform; };
	virtual void Load( const JSON::Vector& Objects ) override;

public:
	CMesh* Mesh;
	CShader* Shader;
	CTexture* Texture;
	CRenderable* Renderable;

	FTransform Transform;
};
