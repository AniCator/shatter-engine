// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glad/include/glad/glad.h>
// #include <ThirdParty/glfw-3.2.1.bin.WIN64/include/GLFW//glfw3.h>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Display/Rendering/Mesh.h>
class CShader;
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Display/Rendering/Uniform.h>

#include <Engine/Utility/Math.h>

struct FRenderData
{
	GLuint VertexBufferObject = 0;
	GLuint IndexBufferObject = 0;
	GLuint ShaderProgram = 0;

	FTransform Transform;
	glm::vec4 Color;

	EDrawMode DrawMode = None;

	bool ShouldRender;
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
	virtual ~CRenderable();

	CMesh* GetMesh();
	void SetMesh( CMesh* Mesh );

	CShader* GetShader();
	void SetShader( CShader* Shader );

	CTexture* GetTexture( ETextureSlot Slot );
	void SetTexture( CTexture* Texture, ETextureSlot Slot );

	void SetUniform( const std::string& Name, const FUniform& Uniform );

	virtual void Draw( FRenderData& RenderData, const FRenderData& PreviousRenderData, EDrawMode DrawModeOverride = None );

	FRenderDataInstanced& GetRenderData();
private:
	CTexture* Textures[TextureSlots];
	CShader* Shader;
	CMesh* Mesh;

	void Prepare( FRenderData& RenderData );

	FRenderDataInstanced RenderData;
	UniformMap Uniforms;
};
