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
	virtual ~CMeshEntity();

	void Spawn( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform );

	virtual void Construct();
	virtual void Tick();
	virtual void Destroy();

	virtual const FTransform& GetTransform() const { return Transform; };

private:
	CMesh* Mesh;
	CShader* Shader;
	CTexture* Texture;
	CRenderable* Renderable;

	FTransform Transform;
};
