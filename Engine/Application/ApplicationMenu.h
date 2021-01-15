// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

bool MenuItem( const char* Label, bool* Selected );

bool* DisplayAssets();
void AssetUI();

bool* DisplayStrings();
void StringUI();

void SetPreviewTexture( class CTexture* Texture );
void SetMouseWheel( const float& Wheel );
