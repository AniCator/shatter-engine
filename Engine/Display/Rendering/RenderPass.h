// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <vector>
#include <unordered_map>

#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>

#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Display/Rendering/Uniform.h>
#include <Engine/Display/Rendering/UniformBuffer.h>

#include <Engine/Utility/Math.h>

class CRenderTexture;

struct RenderPassUniformBlock
{
	glm::mat4 View;
	glm::mat4 Projection;

	Vector3D CameraPosition;
	Vector3D CameraDirection;

	float CameraNear;
	float CameraFar;

	Vector3D PreviousCameraPosition;
	Vector3D PreviousCameraDirection;

	Vector4D Viewport;
};

void PushDebugGroup( const char* Name, const int Length );
void PopDebugGroup();

class CRenderPass
{
public:
	CRenderPass(const std::string& Name, int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true );

	virtual uint32_t RenderRenderable( CRenderable* Renderable );
	virtual uint32_t RenderRenderable( CRenderable* Renderable, UniformMap& Uniforms );
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables );
	virtual uint32_t Render( const std::vector<CRenderable*>& Renderables, UniformMap& Uniforms );

	virtual uint32_t Render( UniformMap& Uniforms );

	void ClearTarget();
	virtual void Clear();

	virtual void Begin();
	virtual void End();

	virtual void Setup( CRenderable* Renderable, UniformMap& Uniforms );
	virtual void Draw( CRenderable* Renderable );
	void SetCamera( const CCamera& Camera );
	void SetPreviousCamera( const CCamera& Camera );

	void UpdateUniformBufferObject();
	void BindUniforms( CRenderable* Renderable, UniformMap& AdditionalUniforms );

	static void FrustumCull( const CCamera& Camera, const std::vector<CRenderable*>& Renderables );

	CRenderTexture* Target;
	CCamera Camera;
	CRenderable* PreviousRenderable = nullptr;
	CCamera PreviousCamera;

	int ViewportWidth;
	int ViewportHeight;

	uint32_t Calls;

	bool AlwaysClear;
	bool SendQueuedRenderables;

	EBlendMode::Type BlendMode;
	EDepthMask::Type DepthMask;
	EDepthTest::Type DepthTest;
	uint8_t StencilValue;

	UniformBufferObject<RenderPassUniformBlock> Block;

protected:
	std::string PassName;

	void ConfigureBlendMode( CShader* Shader );
	void ConfigureDepthMask( CShader* Shader );
	void ConfigureDepthTest( CShader* Shader );
};

uint32_t CopyTexture( CRenderTexture* Source, CRenderTexture* Target, UniformMap& Uniforms );
uint32_t DownsampleTexture( CRenderTexture* Source, CRenderTexture* Target, int Width, int Height, const CCamera& Camera, const bool AlwaysClear, UniformMap& Uniforms );
