// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <ThirdParty/glad/include/glad/glad.h>
// #include <ThirdParty/glfw-3.3.2.bin.WIN64/include/GLFW//glfw3.h>
#include <ThirdParty/glm/glm.hpp>

#include <Engine/Display/Rendering/Mesh.h>
class CShader;
#include <Engine/Display/Rendering/Light/Light.h>
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Display/Rendering/Uniform.h>

#include <Engine/Utility/Math.h>

struct FRenderData
{
	GLuint VertexBufferObject = 0;
	GLuint IndexBufferObject = 0;
	GLuint ShaderProgram = 0;

	FTransform Transform{};
	BoundingBox WorldBounds{};
	Vector4D Color = Vector4D( 1.0f, 1.0f, 1.0f, 1.0f );
	LightIndices LightIndex;

	EDrawMode DrawMode = None;

	bool ShouldRender = false;
	bool DisableRender = false;
	bool DoubleSided = false;
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

	void ApplyMaterial();

	UniformMap GetUniforms() const
	{
			return Uniforms;
	}

	void SetUniform( const std::string& Name, const Uniform& Uniform );

	virtual void Draw( FRenderData& RenderData, const CRenderable* PreviousRenderable, EDrawMode DrawModeOverride = None );

	FRenderDataInstanced& GetRenderData();
	FRenderDataInstanced GetRenderData() const;

	bool HasSkeleton = false;

	static void FrustumCull( const class CCamera& Camera, CRenderable* Renderable );
	static void FrustumCull( const class CCamera& Camera, const std::vector<CRenderable*>& Renderables );

protected:
	CTexture* Textures[TextureSlots];
	CShader* Shader;
	CMesh* Mesh;

	void Prepare( FRenderData& RenderData, const CRenderable* PreviousRenderable );

	// Checks if the renderable is using the same textures.
	bool ShouldBindTextures( const CRenderable* PreviousRenderable );

	// Uniform location
	int TextureLocation[TextureSlots];

	FRenderDataInstanced RenderData;
	UniformMap Uniforms;

	// Individual uniforms.
	Uniform ObjectPosition = { "ObjectPosition" };
	Uniform LightIndices = { "LightIndices" };
	Uniform ObjectBoundsMinimum = { "ObjectBoundsMinimum" };
	Uniform ObjectBoundsMaximum = { "ObjectBoundsMaximum" };
	Uniform ModelMatrix = { "Model" };
	Uniform ObjectColor = { "ObjectColor" };
};
