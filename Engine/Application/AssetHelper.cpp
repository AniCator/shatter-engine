// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "AssetHelper.h"

#include <Engine/Application/Application.h>
#include <Engine/Application/FileDialog.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/Utility/TranslationTable.h>

static auto TranslateAsset = Translate<std::string, EAsset::Type>( {
	{ "Sequence", EAsset::Sequence },
	{ "Sound", EAsset::Sound },
	{ "Texture", EAsset::Texture }
	} 
);

void SelectAssign( std::string& Target, const std::string& Label )
{
	if( ImGui::Selectable( Label.c_str() ) )
	{
		Target = Label;
	}
}

void Assignable( std::string& Target, const std::vector<std::string>& Labels )
{
	for( const auto& Label : Labels )
	{
		SelectAssign( Target, Label );
	}
}

struct AssetTexture
{
	char Location[512] = {};
	std::string Format = "RGB8";
	bool MipMaps = true;
};

void ShowAssetTexture( AssetTexture& Texture )
{
	ImGui::Text( "Location" );
	ImGui::NextColumn();
	ImGui::InputText( "##AssetLocation", Texture.Location, 512 );
	ImGui::SameLine();
	if( ImGui::Button( "..." ) )
	{
		DialogFormats Formats;
		Formats.insert_or_assign( L"Texture", L"*.png;*.jpg;*.tga;" );
		const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
		strcpy_s( Texture.Location, Path.c_str() );
	}

	ImGui::NextColumn();

	// Texture format selection.
	ImGui::Text( "Format" );
	ImGui::NextColumn();

	if( ImGui::BeginCombo( "##TextureFormat", Texture.Format.c_str() ) )
	{
		Assignable( Texture.Format, {
				"R8",
				"RG8",
				"RGB8",
				"RGBA8",
				"R16",
				"RG16",
				"RGB16",
				"RGBA16",
				"R16F",
				"RG16F",
				"RGB16F",
				"RGBA16F",
				"R32F",
				"RG32F",
				"RGB32F",
				"RGBA32F"
			} 
		);

		ImGui::EndCombo();
	}

	ImGui::NextColumn();

	// Mipmaps
	// The origin of the term mipmap is an initialism of the Latin phrase multum in parvo ("much in a small space").
	ImGui::Text( "Mip-Map");
	ImGui::NextColumn();
	ImGui::Checkbox( "##MipMap", &Texture.MipMaps );

	ImGui::NextColumn();
}

bool CreateTexture( const AssetNewData& Data, const AssetTexture& Texture )
{
	const auto Format = CAssets::GetImageFormatFromString( Texture.Format );

	auto& Assets = CAssets::Get();

	if( Assets.FindTexture( Data.Name ) != Assets.FindTexture( "error" ) )
		return false;

	if( Assets.CreateNamedTexture( Data.Name.c_str(), Texture.Location, EFilteringMode::Linear, Format, Texture.MipMaps ) )
		return true;

	return false;
}

struct AssetSound
{
	char Location[512];
	bool Stream;
};

void ShowAssetSound( AssetSound& Sound )
{
	ImGui::Text( "Location" );
	ImGui::NextColumn();
	ImGui::InputText( "##AssetLocation", Sound.Location, 512 );
	ImGui::SameLine();
	if( ImGui::Button( "..." ) )
	{
		DialogFormats Formats;
		Formats.insert_or_assign( L"Audio", L"*.ogg;*.flac;*.wav;" );
		const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
		strcpy_s( Sound.Location, Path.c_str() );
	}

	ImGui::NextColumn();

	ImGui::Text( "Stream" );
	ImGui::NextColumn();

	ImGui::Checkbox( "##AudioStream", &Sound.Stream );
}

bool CreateSound( const AssetNewData& Data, const AssetSound& Sound )
{
	auto& Assets = CAssets::Get();
	if( Assets.Sounds.Find( Data.Name ) )
		return false;

	if( Sound.Stream )
	{
		if( Assets.CreateNamedStream( Data.Name.c_str(), Sound.Location ) )
			return true;
	}
	else
	{
		if( Assets.CreateNamedSound( Data.Name.c_str(), Sound.Location ) )
			return true;
	}

	return false;
}

struct AssetSequence
{
	std::string Location;
};

void ShowAssetSequence( AssetSequence& Data )
{
	ImGui::Text( "Location" );
	ImGui::NextColumn();

	const auto Exists = CFile::Exists( Data.Location );
	if( Exists )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 32, 255, 32, 255 ) );
	}

	ImGui::Text( Data.Location.c_str() );

	if( Exists )
	{
		ImGui::PopStyleColor();
	}

	ImGui::NextColumn();
}

bool CreateSequence( const AssetNewData& Data, const AssetSequence& Sequence )
{
	auto& Assets = CAssets::Get();
	if( Assets.Sequences.Find( Data.Name ) )
		return false;

	if( !CFile::Exists( Sequence.Location ) )
	{
		// Try to generate the file.
		CSequence Dummy;
		Dummy.Save( Sequence.Location.c_str() );

		// Load it again to get the file path saved.
		Dummy.Load( Sequence.Location.c_str() );
		Dummy.Save();
	}

	auto* Asset = Assets.CreateNamedSequence( Data.Name.c_str(), Sequence.Location.c_str() );
	if( !Asset )
		return false;

	// Open the sequencer window.
	Asset->Draw();

	return true;
}

bool DoesAssetExist( const std::string& Name, const EAsset::Type& Type )
{
	const auto& Assets = CAssets::Get();
	switch( Type )
	{
	case EAsset::Mesh:
		return Assets.Meshes.Find( Name ) != nullptr;
	case EAsset::Shader:
		return Assets.Shaders.Find( Name ) != nullptr;
	case EAsset::Texture:
		return Assets.FindTexture( Name ) != Assets.FindTexture( "error" ); // Check for error texture since that is returned by default.
	case EAsset::Sound:
		return Assets.Sounds.Find( Name ) != nullptr;
	case EAsset::Sequence:
		return Assets.Sequences.Find( Name ) != nullptr;
	default:
		return Assets.FindAsset( Name ) != nullptr;
	}
}

void ShowAssetNew( PopupWindow& Window, AssetNewData& Data )
{
	if( Data.Type.empty() )
	{
		Data.Type = "Sound";
	}

	ImGui::BeginChild( "Type Selector", ImVec2( 200.0f, 100.0f ) );
	if( ImGui::BeginCombo( "##Type", Data.Type.c_str() ) )
	{
		if( ImGui::Selectable( "Sequence" ) )
		{
			Data.Type = "Sequence";
		}

		ImGui::Separator();

		if( ImGui::Selectable( "Sound" ) )
		{
			Data.Type = "Sound";
		}

		ImGui::Separator();

		if( ImGui::Selectable( "Texture" ) )
		{
			Data.Type = "Texture";
		}

		ImGui::Separator();

		ImGui::EndCombo();
	}

	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild( "Asset Editing", ImVec2( 400.0f, 100.0f ), false, ImGuiWindowFlags_AlwaysAutoResize );
	const auto Type = TranslateAsset.To( Data.Type );

	ImGui::Columns( 2 );

	static char AssetName[128];
	ImGui::Text( "Name" );
	ImGui::NextColumn();

	const auto Exists = DoesAssetExist( AssetName, Type );
	if( Exists ) 
	{
		ImGui::PushStyleColor( ImGuiCol_FrameBg, IM_COL32( 127, 0, 32, 255 ) );
	}

	if( ImGui::InputText( "##Name", AssetName, 128 ) )
	{
		auto AssetString = std::string( AssetName );
		std::transform( AssetString.begin(), AssetString.end(), AssetString.begin(), ::tolower );
		strcpy_s( AssetName, 128, AssetString.c_str() );
	}

	const auto EmptyNameField = std::string( AssetName ).empty();

	if( Exists )
	{
		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			ImGui::Text( "This name is already in use for this asset type." );
			ImGui::EndTooltip();
		}

		ImGui::PopStyleColor();
	}

	ImGui::SetItemDefaultFocus();
	ImGui::NextColumn();

	static AssetTexture TextureData;
	static AssetSound SoundData;
	static AssetSequence SequenceData;
	switch( Type )
	{
	case EAsset::Mesh:
		break;
	case EAsset::Shader: 
		break;
	case EAsset::Texture:
		ShowAssetTexture( TextureData );
		break;
	case EAsset::Sound:
		ShowAssetSound( SoundData );
		break;
	case EAsset::Sequence:
		SequenceData.Location = "Sequences/" + std::string( AssetName ) + ".lsq";
		ShowAssetSequence( SequenceData );
		break;
	default: ;
	}

	ImGui::EndChild();
	ImGui::BeginChild( "Asset Buttons", ImVec2( ImGui::GetContentRegionAvail().x, 30.0f ) );
	if( ImGui::Button( "Create" ) )
	{
		bool Success = false;

		Data.Name = AssetName;
		if( !Data.Name.empty() )
		{
			switch( Type )
			{
			case EAsset::Mesh: 
				break;
			case EAsset::Shader: 
				break;
			case EAsset::Texture:
				Success = CreateTexture( Data, TextureData );
				break;
			case EAsset::Sound:
				Success = CreateSound( Data, SoundData );
				break;
			case EAsset::Sequence:
				Success = CreateSequence( Data, SequenceData );
				break;
			default:;
			}
		}

		if( Success )
		{
			Window.Open = false;
			ImGui::CloseCurrentPopup();

			// Clear all the temporary asset data.
			memset( AssetName, 0, 128 );
			// Data = AssetNewData();
			TextureData = AssetTexture();
			SoundData = AssetSound();
			SequenceData = AssetSequence();
		}
		else
		{
			ImGui::SameLine();
			ImGui::Text( "Something went wrong.\n" );
		}
	}
	ImGui::EndChild();
}

void AssetNew( PopupWindow& Window, AssetNewData& Data )
{
	static uint32_t TitleID = 0;
	Window.Title = "Import Asset##ImportAssetWindow" + std::to_string( TitleID++ );
	Window.Function = [&] ()
	{
		ShowAssetNew( Window, Data );
	};
}

void OpenAssetWindow( PopupWindow& Window )
{
	Window.Open = true;
	ImGui::OpenPopup( Window.Title.c_str() );
}

void CloseAssetWindow( PopupWindow& Window )
{
	Window.Open = false;
}

void CreateAssetWindow( AssetNewPopup& Popup )
{
	if( Popup.Created )
		return;

	AssetNew( Popup.Window, Popup.Data );
	Popup.Created = true;
}

void DisplayAssetWindow( AssetNewPopup& Popup )
{
	CreateAssetWindow( Popup );

	const bool IsOpen = ImGui::BeginPopupModal( Popup.Window.Title.c_str(), &Popup.Window.Open, ImGuiWindowFlags_AlwaysAutoResize );
	if( IsOpen )
	{
		Popup.Window.Function();
		ImGui::EndPopup();
	}

	if( !IsOpen && Popup.Window.Open )
	{
		Popup.Window.Open = IsOpen;
	}

	if( IsOpen && !Popup.Window.Open )
	{
		ImGui::CloseCurrentPopup();
	}
}

template<typename T>
void DisplayDropdown( const std::unordered_map<std::string, T*>& Assets, AssetDropdownData& Data )
{
	for( auto& Pair : Assets )
	{
		if( !Data.Filter.PassFilter( Pair.first.c_str() ) )
			continue;

		ImGui::SetCursorPosX( 10.0f );

		if( ImGui::Selectable( Pair.first.c_str(), false, ImGuiSelectableFlags_None, 
			ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 0.0f) ) )
		{
			Data.Name = Pair.first;
			Data.Asset = Pair.second;
		}

		ImGui::Separator();
	}
}

template<typename T>
void DisplayDropdown( const AssetPool<T>& Assets, AssetDropdownData& Data )
{
	for( auto& Pair : Assets.Get() )
	{
		if( !Data.Filter.PassFilter( Pair.first.c_str() ) )
			continue;

		ImGui::SetCursorPosX( 10.0f );

		if( ImGui::Selectable( Pair.first.c_str(), false, ImGuiSelectableFlags_None,
			ImVec2( ImGui::GetContentRegionAvail().x - 10.0f, 0.0f ) ) )
		{
			Data.Name = Pair.first;
			Data.Asset = Assets.Get( Pair.second );
		}

		ImGui::Separator();
	}
}

void DisplayDropdown( const EAsset::Type& Type, AssetDropdownData& Data )
{
	const auto& Assets = CAssets::Get();
	switch( Type )
	{
	case EAsset::Mesh:
		DisplayDropdown( Assets.Meshes, Data );
		break;
	case EAsset::Shader:
		DisplayDropdown( Assets.Shaders, Data );
		break;
	case EAsset::Texture:
		DisplayDropdown( Assets.Textures, Data );
		break;
	case EAsset::Sound:
		DisplayDropdown( Assets.Sounds, Data );
		break;
	case EAsset::Sequence:
		DisplayDropdown( Assets.Sequences, Data );
		break;
	case EAsset::Generic:
		DisplayDropdown( Assets.Assets, Data );
		break;
	default:;
	}
}

template<typename T>
void AssignAsset( const std::unordered_map<std::string, T*>& Assets, AssetDropdownData& Data )
{
	// Check if the asset is assigned correctly.
	if( !Data.Name.empty() && !Data.Asset )
	{
		const auto Iterator = Assets.find( Data.Name );
		if( Iterator != Assets.end() )
		{
			Data.Asset = Iterator->second;
		}
	}
}

template<typename T>
void AssignAsset( const AssetPool<T>& Assets, AssetDropdownData& Data )
{
	// Check if the asset is assigned correctly.
	if( !Data.Name.empty() && !Data.Asset )
	{
		const auto Asset = Assets.Find( Data.Name );
		if( Asset != nullptr )
		{
			Data.Asset = Asset;
		}
	}
}

void AssignAsset( const EAsset::Type& Type, AssetDropdownData& Data )
{
	if( Data.Clear )
		return;

	const auto& Assets = CAssets::Get();
	switch( Type )
	{
	case EAsset::Mesh:
		AssignAsset( Assets.Meshes, Data );
		break;
	case EAsset::Shader:
		AssignAsset( Assets.Shaders, Data );
		break;
	case EAsset::Texture:
		AssignAsset( Assets.Textures, Data );
		break;
	case EAsset::Sound:
		AssignAsset( Assets.Sounds, Data );
		break;
	case EAsset::Sequence:
		AssignAsset( Assets.Sequences, Data );
		break;
	case EAsset::Generic:
		AssignAsset( Assets.Assets, Data );
		break;
	default:;
	}
}

uint32_t AssetDropdownData::Instances = 0;
void DisplayAssetDropdown( const EAsset::Type& Type, AssetDropdownData& Data )
{
	const auto CurrentName = Data.Name;

	if( Data.Create )
	{
		if( Data.Popup.Window.Open && !ImGui::IsPopupOpen( Data.Popup.Window.Title.c_str() ) )
		{
			OpenAssetWindow( Data.Popup.Window );
		}

		DisplayAssetWindow( Data.Popup );

		if( !Data.Popup.Window.Open )
		{
			Data.Create = false;
			Data.Name = Data.Popup.Data.Name;
			AssignAsset( Type, Data );
		}
	}

	const auto DropDownName = "##AssetDropDown" + Data.Popup.Window.Title + std::to_string( Data.Identifier );
	if( ImGui::BeginCombo( DropDownName.c_str(), Data.PreviewName.c_str() ) )
	{
		ImGui::Separator();
		ImGui::Separator();
		ImGui::SetCursorPosX( 10.0f );

		if( Data.OpenedTicks < 10 )
		{
			ImGui::SetKeyboardFocusHere( 0 );
			Data.Filter.Clear();
		}

		Data.OpenedTicks++;

		Data.Filter.Draw( "", 300.0f );

		ImGui::SetItemDefaultFocus();

		ImGui::SameLine();
		if( ImGui::SmallButton( "X##ClearAsset" ) )
		{
			Data.Clear = true;

			ImGui::CloseCurrentPopup();
		}

		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			ImGui::Text( "Clear Asset" );
			ImGui::EndTooltip();
		}

		ImGui::SameLine();
		if( ImGui::SmallButton( "+##AddAsset" ) )
		{
			Data.Popup = AssetNewPopup();
			Data.Create = true;
			Data.Popup.Window.Open = true;
			Data.Popup.Data.Type = TranslateAsset.From( Type );
			CreateAssetWindow( Data.Popup );
		}

		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			ImGui::Text( "Import Asset" );
			ImGui::EndTooltip();
		}

		ImGui::Separator();
		ImGui::Separator();

		// TODO: Handle edge case of asset existing but not being assigned.
		// AssignAsset( Type, Data );
		DisplayDropdown( Type, Data );

		ImGui::EndCombo();
	}
	else
	{
		Data.OpenedTicks = 0;
	}

	const auto NameChanged = CurrentName != Data.Name;
	const auto ShouldUpdatePreviewName = !Data.Name.empty() && Data.Name != Data.PreviewName;
	if( NameChanged || ShouldUpdatePreviewName )
	{
		Data.PreviewName = Data.Name;
	}
}
