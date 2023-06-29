// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderable.h"

#include <Engine/Display/Rendering/Culling.h>
#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/Entity/LightEntity/LightEntity.h>

#include <ThirdParty/glm/gtc/type_ptr.hpp>

GLuint DummyVAO = 0;
void BindDummyVAO()
{
	if( DummyVAO == 0 )
	{
		glGenVertexArrays( 1, &DummyVAO );
	}

	glBindVertexArray( DummyVAO );
}

CRenderable::CRenderable()
{
	Mesh = nullptr;
	Shader = nullptr;
	memset( Textures, 0, 32 * sizeof( CTexture* ) );

	for( int& Index : TextureLocation )
	{
		Index = -1;
	}

	RenderData.ShouldRender = true;
}

CRenderable::~CRenderable()
{

}

CMesh* CRenderable::GetMesh()
{
	return Mesh;
}


void CRenderable::SetMesh( CMesh* Mesh )
{
	if( Mesh && this->Mesh != Mesh )
	{
		this->Mesh = Mesh;

		FVertexBufferData& Data = Mesh->GetVertexBufferData();
		RenderData.VertexBufferObject = Data.VertexBufferObject;
		RenderData.IndexBufferObject = Data.IndexBufferObject;
	}
}

CShader* CRenderable::GetShader()
{
	return Shader;
}

void CRenderable::SetShader( CShader* Shader )
{
	if( !Shader )
		return;

	if( this->Shader == Shader )
		return;

	this->Shader = Shader;
	CheckCachedUniforms();
}

CTexture* CRenderable::GetTexture( ETextureSlot Slot )
{
	if( Slot < ETextureSlot::Maximum )
	{
		const auto Index = static_cast<ETextureSlotType>( Slot );
		return Textures[Index];
	}

	return nullptr;
}

void CRenderable::SetTexture( CTexture* Texture, ETextureSlot Slot )
{
	if( Texture && Slot < ETextureSlot::Maximum )
	{
		const auto Index = static_cast<ETextureSlotType>( Slot );
		this->Textures[Index] = Texture;
	}
}

void CRenderable::SetUniform( const std::string& Name, const Uniform& Uniform )
{
	Uniforms.insert_or_assign( Name, Uniform );
}

void CRenderable::CheckCachedUniforms()
{
	if( !Shader )
		return;

	const auto Handles = Shader->GetHandles();
	if( RenderData.ShaderProgram == Handles.Program )
		return;

	RenderData.ShaderProgram = Handles.Program;

	// Cache the texture locations.
	for( size_t Index = 0; Index < TextureSlots; Index++ )
	{
		TextureLocation[Index] = glGetUniformLocation( Handles.Program, TextureSlotName[Index] );
	}
}

void CRenderable::Draw( FRenderData& RenderData, const CRenderable* PreviousRenderable, EDrawMode DrawModeOverride )
{
	if( !RenderData.ShouldRender )
		return;
	
	const EDrawMode DrawMode = DrawModeOverride != None ? DrawModeOverride : RenderData.DrawMode;
	Prepare( RenderData, PreviousRenderable );

	ObjectPosition.Set( RenderData.Transform.GetPosition() );
	ObjectPosition.Bind( RenderData.ShaderProgram );

	LightIndices.Set( RenderData.LightIndex.Index );
	LightIndices.Bind( RenderData.ShaderProgram );

	if( Mesh )
	{
		auto& AABB = Mesh->GetBounds();

		ObjectBoundsMinimum.Set( AABB.Minimum );
		ObjectBoundsMinimum.Bind( RenderData.ShaderProgram );

		ObjectBoundsMaximum.Set( AABB.Maximum );
		ObjectBoundsMaximum.Bind( RenderData.ShaderProgram );
	}

	ModelMatrix.Set( RenderData.Transform.GetTransformationMatrix() );
	ModelMatrix.Bind( RenderData.ShaderProgram );

	ObjectColor.Set( RenderData.Color );
	ObjectColor.Bind( RenderData.ShaderProgram );

	for( auto& UniformBuffer : Uniforms )
	{
		UniformBuffer.second.Bind( RenderData.ShaderProgram, UniformBuffer.first );
	}

	if( DrawMode == EDrawMode::FullScreenTriangle )
	{
		BindDummyVAO();
		glDrawArrays( GL_TRIANGLES, 0, 3 );
		return;
	}

	if( !Mesh )
		return; // No mesh provided.
	
	bool BindBuffers = true;
	if( PreviousRenderable )
	{
		const FVertexBufferData& Data = Mesh->GetVertexBufferData();
		const auto& PreviousRenderData = PreviousRenderable->RenderData;
		BindBuffers = PreviousRenderData.VertexBufferObject != Data.VertexBufferObject || 
			PreviousRenderData.IndexBufferObject != Data.IndexBufferObject;
	}

	if( BindBuffers )
	{
		Mesh->Prepare( DrawMode );
	}

	const bool UpdateFaceCulling = PreviousRenderable ? PreviousRenderable->RenderData.DoubleSided != RenderData.DoubleSided : true;
	const bool IsFlipped = RenderData.Transform.GetSize().X < 0.0f || RenderData.Transform.GetSize().Y < 0.0f || RenderData.Transform.GetSize().Z < 0.0f;

	// Check if we need to update face culling first, otherwise check if the geometry is flipped.
	if( UpdateFaceCulling )
	{
		if( RenderData.DoubleSided )
		{
			// Disable face culling.
			glDisable( GL_CULL_FACE );
		}
		else
		{
			// Enable face culling.
			glEnable( GL_CULL_FACE );
		}
	}
	else if ( IsFlipped )
	{
		// Flip the winding order.
		glCullFace( GL_FRONT );
	}

	Mesh->Draw( DrawMode );

	if( IsFlipped )
	{
		glCullFace( GL_BACK );
	}
}

FRenderDataInstanced& CRenderable::GetRenderData()
{
	return RenderData;
}

FRenderDataInstanced CRenderable::GetRenderData() const
{
	return RenderData;
}

void CRenderable::FrustumCull( const CCamera& Camera, CRenderable* Renderable )
{
	Culling::Frustum( Camera, Renderable );
}

void CRenderable::FrustumCull( const CCamera& Camera, const std::vector<CRenderable*>& Renderables )
{
	Culling::Frustum( Camera, Renderables );
}

static const GLenum StencilTestToEnum[EStencilTest::Maximum]
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

void CheckStencil( 
	bool& StencilTest,
	bool& StencilValue,
	bool& StencilWrite,
	const FRenderData& RenderData, const CRenderable* PreviousRenderable 
)
{
	if( !PreviousRenderable )
		return;

	const auto& PreviousRenderData = PreviousRenderable->GetRenderData();
	if( RenderData.StencilTest == PreviousRenderData.StencilTest )
	{
		// Stencil tests match, doesn't require an update.
		StencilTest = false;
	}

	if( RenderData.StencilValue == PreviousRenderData.StencilValue )
	{
		// Stencil values match, doesn't require an update.
		StencilValue = false;
	}

	if( RenderData.StencilWrite == PreviousRenderData.StencilWrite )
	{
		// Stencil write operations match, doesn't require an update.
		StencilWrite = false;
	}
}

void ConfigureStencil( const FRenderData& RenderData, const CRenderable* PreviousRenderable )
{
	bool StencilTest = true;
	bool StencilValue = true;
	bool StencilWrite = true;
	CheckStencil( StencilTest, StencilValue, StencilWrite, RenderData, PreviousRenderable );

	if( StencilTest || StencilValue )
	{
		glStencilFunc( StencilTestToEnum[RenderData.StencilTest], RenderData.StencilValue, 255 );
	}

	if( StencilWrite )
	{
		if( RenderData.StencilWrite )
		{
			glStencilMask( 255 );
		}
		else
		{
			glStencilMask( 0 );
		}
	}
}

void CRenderable::Prepare( FRenderData& RenderData, const CRenderable* PreviousRenderable )
{
	if( !Shader )
		return;

	CheckCachedUniforms();
	ConfigureStencil( RenderData, PreviousRenderable );

	if( !ShouldBindTextures( PreviousRenderable ) )
		return;

	for( ETextureSlot Slot = ETextureSlot::Slot0; Slot < ETextureSlot::Maximum; )
	{
		const auto Index = static_cast<ETextureSlotType>( Slot );

		CTexture* Texture = Textures[Index];
		if( Texture )
		{
			Texture->Bind( Slot );
		}

		const auto Location = TextureLocation[Index];
		if( Location > -1 )
		{
			glUniform1i( Location, Index );
		}

		Slot = static_cast<ETextureSlot>( Index + 1 );
	}
}

bool CRenderable::ShouldBindTextures( const CRenderable* PreviousRenderable )
{
	if( !Textures[0] )
		return false;

	// Check if the previous renderable was using the same textures.
	bool TextureMatch = true;
	if( PreviousRenderable )
	{
		for( ETextureSlot Slot = ETextureSlot::Slot0; Slot < ETextureSlot::Maximum; )
		{
			const auto Index = static_cast<ETextureSlotType>( Slot );
			const auto* TextureA = Textures[Index];
			const auto* TextureB = PreviousRenderable->Textures[Index];
			if( TextureA != TextureB )
			{
				TextureMatch = false;
				break;
			}

			Slot = static_cast<ETextureSlot>( Index + 1 );
		}
	}
	else
	{
		TextureMatch = false;
	}

	return !TextureMatch;
}
