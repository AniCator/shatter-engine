// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ImageEvent.h"

#include <Engine/Display/Rendering/Pass/PostProcessPass.h>
#include <Engine/Display/Window.h>
#include <Engine/Resource/Assets.h>
#include <Engine/World/World.h>

#include <ThirdParty/imgui-1.70/imgui.h>

void FEventImage::Execute()
{
	if( !Texture )
		return;
	
	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return;
	
	auto& Window = CWindow::Get();	
	static CRenderPassPostProcess ImageEventPass( Window.GetWidth(), Window.GetHeight(), *World->GetActiveCamera() );
	ImageEventPass.ViewportWidth = Window.GetWidth();
	ImageEventPass.ViewportHeight = Window.GetHeight();
	ImageEventPass.SetTexture( Texture );

	Window.GetRenderer().AddRenderPass( &ImageEventPass, ERenderPassLocation::Standard );
}

void FEventImage::Context()
{
	ImGui::Text( "Image" );
	ImGui::Text( "%s", Name.length() > 0 && Texture != nullptr ? Name.c_str() : "no texture selected" );

	ImGui::Separator();
	int InputLength = StaticCast<int>( Length );
	if( ImGui::InputInt( "Length", &InputLength, Timebase / 4 ) )
	{
		Length = StaticCast<Timecode>( InputLength );
	}

	ImGui::Separator();

	if( ImGui::BeginCombo( "##TextureAssets", Name.c_str() ) )
	{
		auto& Assets = CAssets::Get();
		for( const auto& Pair : Assets.Textures.Get() )
		{
			if( ImGui::Selectable( Pair.first.c_str() ) )
			{
				Name = Pair.first;
				Texture = Assets.Textures.Get( Pair.second );
			}

			ImGui::Separator();
		}

		ImGui::EndCombo();
	}
}

const char* FEventImage::GetName()
{
	return Name.c_str();
}

const char* FEventImage::GetType() const
{
	return "Image";
}

void FEventImage::Export( CData& Data ) const
{
	TrackEvent::Export( Data );

	DataString::Encode( Data, Name );

	if( Texture )
	{
		DataString::Encode( Data, Texture->GetLocation() );
	}
	else
	{
		DataMarker::Mark( Data, "NOTEXTURE" );
	}
}

void FEventImage::Import( CData& Data )
{
	TrackEvent::Import( Data );

	DataString::Decode( Data, Name );

	if( DataMarker::Check( Data, "NOTEXTURE" ) )
	{
		Texture = nullptr;
	}
	else
	{
		std::string FileLocation;
		DataString::Decode( Data, FileLocation );
		Texture = CAssets::Get().CreateNamedTexture( Name.c_str(), FileLocation.c_str() );
	}
}
