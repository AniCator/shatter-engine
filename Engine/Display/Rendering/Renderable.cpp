// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "Renderable.h"

#include <Engine/Display/Rendering/Shader.h>

CRenderable::CRenderable()
{
	Mesh = nullptr;
	Shader = nullptr;
	memset( Textures, 0, 32 * sizeof( CTexture* ) );
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
		RenderData.ShaderProgram = Shader->Handle;
	}
}

CTexture* CRenderable::GetTexture( ETextureSlot Slot )
{
	if( Slot < ETextureSlot::Maximum )
	{
		const auto Index = static_cast<std::underlying_type<ETextureSlot>::type>( Slot );
		return Textures[Index];
	}

	return nullptr;
}

void CRenderable::SetTexture( CTexture* Texture, ETextureSlot Slot )
{
	if( Texture && Slot < ETextureSlot::Maximum )
	{
		const auto Index = static_cast<std::underlying_type<ETextureSlot>::type>( Slot );
		this->Textures[Index] = Texture;
	}
}

void CRenderable::Draw( const FRenderData& PreviousRenderData, EDrawMode DrawModeOverride )
{
	if( Mesh )
	{
		const EDrawMode DrawMode = DrawModeOverride != None ? DrawModeOverride : RenderData.DrawMode;
		Prepare();

		const FVertexBufferData& Data = Mesh->GetVertexBufferData();
		const bool BindBuffers = PreviousRenderData.VertexBufferObject != Data.VertexBufferObject || PreviousRenderData.IndexBufferObject != Data.IndexBufferObject;
		if( BindBuffers )
		{
			Mesh->Prepare( DrawMode );
		}

		Mesh->Draw( DrawMode );
	}
}

FRenderDataInstanced& CRenderable::GetRenderData()
{
	return RenderData;
}

void CRenderable::Prepare()
{
	if( Shader )
	{
		if( Textures )
		{
			for( ETextureSlot Slot = ETextureSlot::Slot0; Slot < ETextureSlot::Maximum; )
			{
				const auto Index = static_cast<std::underlying_type<ETextureSlot>::type>( Slot );

				char UniformName[32];
				sprintf_s( UniformName, "Texture%i", Index );
				glUniform1i( glGetUniformLocation( Shader->Handle, UniformName ), Index );

				CTexture* Texture = Textures[Index];
				if( Texture )
				{
					Texture->Bind( Slot );
				}

				Slot = static_cast<ETextureSlot>( Index + 1 );
			}
		}
	}
}
