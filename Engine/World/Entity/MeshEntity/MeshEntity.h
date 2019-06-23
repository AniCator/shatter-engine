// Copyright � 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/World/Entity/PointEntity/PointEntity.h>
#include <Engine/Utility/Math.h>

class CMesh;
class CShader;
class CTexture;
class CRenderable;

class CMeshEntity : public CPointEntity
{
public:
	CMeshEntity();
	CMeshEntity( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform );
	virtual ~CMeshEntity() override;

	virtual void Spawn( CMesh* Mesh, CShader* Shader, CTexture* Texture, FTransform& Transform );

	virtual void Construct() override;
	virtual void Tick() override;
	virtual void Destroy() override;

	virtual void Debug() override;

	virtual void Load( const JSON::Vector& Objects ) override;
	virtual void Reload() override;

	virtual void Import( CData& Data ) override;
	virtual void Export( CData& Data ) override;

public:
	CMesh* Mesh;
	CShader* Shader;
	CTexture* Texture;
	CRenderable* Renderable;

	glm::vec4 Color;

	std::string MeshName;
	std::string ShaderName;
	std::string TextureName;
};
