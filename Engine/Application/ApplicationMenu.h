// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <Engine/Audio/SoLoud/Bus.h>

bool MenuItem( const char* Label, bool* Selected );

bool* DisplayAssets();
bool* DisplayStrings();

void SetPreviewTexture( class CTexture* Texture );
void SetMouseWheel( const float& Wheel );

CTexture* GetThumbnail( const std::string& Name );
CTexture* GenerateThumbnail( const std::string& Name, class CMesh* Mesh, class MaterialAsset* Material = nullptr, const float FieldOfView = 55.0f );

bool* DisplayMixer();
bool* DisplayShaderToy();

void RenderCommandItems();

void RenderWindowItems();
void RenderWindowPanels();

void PopulateGradePanel( struct ColorGrade& Grade );

namespace ImGui
{
	Bus::Type BusSelector( const Bus::Type& Bus );
}
