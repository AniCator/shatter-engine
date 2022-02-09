// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <functional>
#include <string>

#include <ThirdParty/imgui-1.70/imgui.h>

#include <Engine/Resource/Assets.h>

struct PopupWindow
{
	std::string Title;
	bool Open = false;
	std::function<void()> Function;
};

struct AssetNewData
{
	std::string Type;
	std::string Name;
};

struct AssetNewPopup
{
	PopupWindow Window;
	AssetNewData Data;
	bool Created = false;
};

void OpenAssetWindow( PopupWindow& Window );
void CloseAssetWindow( PopupWindow& Window );

void DisplayAssetWindow( AssetNewPopup& Popup );

struct AssetDropdownData
{
	std::string Name;
	void* Asset = nullptr;

	std::string PreviewName;

	ImGuiTextFilter Filter;
	uint32_t OpenedTicks = 0;

	// The user has pressed the clear button.
	bool Clear = false;

	// Used for when the user wants to create a new asset from the dropdown.
	AssetNewPopup Popup;

	// User is creating a new asset.
	bool Create = false;

	template<typename AssetType>
	void Assign( std::string& OutputName, AssetType*& Output )
	{
		if( Asset )
		{
			OutputName = Name;
			Output = static_cast<AssetType*>( Asset );
			*this = AssetDropdownData(); // Clear the data.
			PreviewName = Name;
		}

		if( Clear )
		{
			OutputName = "";
			Output = nullptr;
			*this = AssetDropdownData(); // Clear the data.
		}
	}
};

// Searchable asset drop-down.
void DisplayAssetDropdown( const EAsset::Type& Type, AssetDropdownData& Data );
