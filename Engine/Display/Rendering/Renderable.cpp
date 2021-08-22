// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "Renderable.h"

#include <Engine/Display/Rendering/Shader.h>
#include <Engine/Profiling/Logging.h>
#include <Engine/World/Entity/LightEntity/LightEntity.h>

#include <ThirdParty/glm/gtc/type_ptr.hpp>

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
	if( Mesh )
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
	if( Shader )
	{
		this->Shader = Shader;

		const FProgramHandles& Handles = Shader->GetHandles();
		RenderData.ShaderProgram = Handles.Program;

		for( size_t Index = 0; Index < TextureSlots; Index++ )
		{
			TextureLocation[Index] = glGetUniformLocation( Handles.Program, TextureSlotName[Index] );;
		}
	}
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

void CRenderable::Draw( FRenderData& RenderData, const FRenderData& PreviousRenderData, EDrawMode DrawModeOverride )
{
	if( RenderData.ShouldRender )
	{
		const EDrawMode DrawMode = DrawModeOverride != None ? DrawModeOverride : RenderData.DrawMode;
		Prepare( RenderData );

		const GLint ObjectPositionLocation = glGetUniformLocation( RenderData.ShaderProgram, "ObjectPosition" );
		if( ObjectPositionLocation > -1 )
		{
			glUniform3fv( ObjectPositionLocation, 1, RenderData.Transform.GetPosition().Base() );
		}

		const GLint LightIndicesLocation = glGetUniformLocation( RenderData.ShaderProgram, "LightIndices" );
		if( LightIndicesLocation > -1 )
		{
			glUniform4iv( LightIndicesLocation, 1, RenderData.LightIndex.Index );
		}

		if( Mesh )
		{
			auto& AABB = Mesh->GetBounds();
			const GLint ObjectBoundsMinimumLocation = glGetUniformLocation( RenderData.ShaderProgram, "ObjectBoundsMinimum" );
			if( ObjectBoundsMinimumLocation > -1 )
			{
				glUniform3fv( ObjectBoundsMinimumLocation, 1, AABB.Minimum.Base() );
			}

			const GLint ObjectBoundsMaximumLocation = glGetUniformLocation( RenderData.ShaderProgram, "ObjectBoundsMaximum" );
			if( ObjectBoundsMaximumLocation > -1 )
			{
				glUniform3fv( ObjectBoundsMaximumLocation, 1, AABB.Maximum.Base() );
			}
		}

		const auto ModelMatrix = RenderData.Transform.GetTransformationMatrix();
		const GLuint ModelMatrixLocation = glGetUniformLocation( RenderData.ShaderProgram, "Model" );
		glUniformMatrix4fv( ModelMatrixLocation, 1, GL_FALSE, &ModelMatrix[0][0] );

		const GLuint ColorLocation = glGetUniformLocation( RenderData.ShaderProgram, "ObjectColor" );
		glUniform4fv( ColorLocation, 1, RenderData.Color.Base() );

		for( const auto& UniformBuffer : Uniforms )
		{
			UniformBuffer.second.Bind( RenderData.ShaderProgram, UniformBuffer.first );
		}

		if( Mesh )
		{
			const FVertexBufferData& Data = Mesh->GetVertexBufferData();
			const bool BindBuffers = PreviousRenderData.VertexBufferObject != Data.VertexBufferObject || PreviousRenderData.IndexBufferObject != Data.IndexBufferObject;
			if( BindBuffers )
			{
				Mesh->Prepare( DrawMode );
			}

			Mesh->Draw( DrawMode );
		}
	}
}

FRenderDataInstanced& CRenderable::GetRenderData()
{
	return RenderData;
}

void CRenderable::Prepare( FRenderData& RenderData )
{
	if( Shader )
	{
		const FProgramHandles& Handles = Shader->GetHandles();
		if( Textures[0] )
		{
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
	}
}
