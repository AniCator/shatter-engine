// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <string>
#include <unordered_map>

bool MenuItem( const char* Label, bool* Selected );

bool* DisplayAssets();
bool* DisplayStrings();

void SetPreviewTexture( class CTexture* Texture );
void SetMouseWheel( const float& Wheel );

bool* DisplayMixer();
bool* DisplayShaderToy();

void RenderMenuItems();
void RenderMenuPanels();

typedef std::unordered_map<std::wstring, std::wstring> DialogFormats;
std::string OpenFileDialog( const DialogFormats& Formats );
