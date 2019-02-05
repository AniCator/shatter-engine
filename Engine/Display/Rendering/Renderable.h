// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glad/include/glad/glad.h>
#include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW//glfw3.h>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Display/Rendering/Mesh.h>
class CShader;
#include <Engine/Display/Rendering/Texture.h>

struct FRenderData
{
	FVertexBufferData VertexBufferData;
	GLuint ShaderProgram = 0;

	glm::vec3 Position;
	glm::vec3 Orientation;
	glm::vec3 Size;
	glm::vec4 Color;

	FVertexData *VertexData = nullptr;
	EDrawMode DrawMode = None;
};

struct FRenderDataInstanced : public FRenderData
{
	GLuint PositionBufferObject = 0;
	GLuint ColorBufferObject = 0;
};

class CRenderable
{
public:
	CRenderable();
	~CRenderable();

	CMesh* GetMesh();
	void SetMesh( CMesh* Mesh );

	CShader* GetShader();
	void SetShader( CShader* Shader );

	CTexture* GetTexture( ETextureSlot Slot );
	void SetTexture( CTexture* Texture, ETextureSlot Slot );

	virtual void Draw( EDrawMode DrawModeOverride = None );

	FRenderDataInstanced& GetRenderData();
private:
	CTexture* Textures[static_cast<std::underlying_type<ETextureSlot>::type>( ETextureSlot::Maximum )];
	CShader* Shader;
	CMesh* Mesh;

	FRenderDataInstanced RenderData;
};
