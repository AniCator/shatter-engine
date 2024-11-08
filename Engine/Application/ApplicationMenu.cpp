// Copyright � 2017, Christiaan Bakker, All rights reserved.
#include "ApplicationMenu.h"

#include <Engine/Application/Application.h>
#include <Engine/Application/AssetHelper.h>
#include <Engine/Application/FileDialog.h>
#include <Engine/Audio/Sound.h>
#include <Engine/Audio/SoundInstance.h>
#include <Engine/Audio/SoLoud/EffectStack.h>
#include <Engine/Configuration/Configuration.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Material.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Resource/Asset.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/World/World.h>
#include <Engine/Utility/TranslationTable.h>

#include <Game/Game.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.70/imgui.h>
#endif

CMesh* PreviewMesh = nullptr;
MaterialAsset* PreviewMaterial = nullptr;
CRenderTexture* PreviewRenderTexture;
class CModelPass : public CRenderPass
{
public:
	CModelPass()
		: CRenderPass( "ModelPass", 512, 512, {}, false)
	{
		
	}

	uint32_t Render( UniformMap& Uniforms ) override
	{
		auto* Mesh = Model.GetMesh();
		if( !Mesh )
			return 0;
		
		const auto Bounds = Mesh->GetBounds();
		const auto Center = Bounds.Center();
		const auto Extent = Bounds.Size() * 0.85f;
		const auto MaximumDistance = Math::Max( Extent );

		const auto Time = StaticCast<float>( GameLayersInstance->GetRealTime() );
		
		auto& Setup = Camera.GetCameraSetup();
		Setup.CameraPosition = Center + Vector3D( MaximumDistance * 2.0f * sin( Time ), MaximumDistance * 2.0f * cos( Time ), MaximumDistance );
		Setup.CameraDirection = Center - Setup.CameraPosition;
		Setup.NearPlaneDistance = 0.01f;
		Setup.FarPlaneDistance = Setup.CameraDirection.Length() * 2.0f;
		Setup.CameraDirection.Normalize();
		Setup.FieldOfView = FieldOfView;
		Setup.AspectRatio = 1.1f;
		Camera.Update();

		auto& RenderData = Model.GetRenderData();
		RenderData.Transform = FTransform();

		ClearTarget();

		RenderRenderable( &Model, Uniforms );

		// Resolve the texture if it was using MSAA.
		if( Target->GetSampleCount() > 1 )
		{
			Target->Resolve();

			// We can sneakily apply the tonemapper with MSAA.
			auto& Assets = CAssets::Get();
			CRenderable ResolveRenderable;
			ResolveRenderable.GetRenderData().DrawMode = FullScreenTriangle;
			ResolveRenderable.SetMesh( nullptr );
			ResolveRenderable.SetShader( Assets.Shaders.Find( "resolve" ) );
			ResolveRenderable.SetTexture( Target, ETextureSlot::Slot0 );
			RenderRenderable( &ResolveRenderable, Uniforms );

			// Resolve the texture again if it was using MSAA.
			if( Target->GetSampleCount() > 1 )
			{
				Target->Resolve();
			}
		}

		return Calls;
	}

	void Reset()
	{
		auto& Assets = CAssets::Get();

		Model = {};
		Model.SetShader( Assets.Shaders.Find( "default" ) );
		Model.SetTexture( Assets.FindTexture( "error" ), ETextureSlot::Slot0 );
	}

	CRenderable Model;
	float FieldOfView = 55.0f;
};

bool MenuItem( const char* Label, bool* Selected )
{
	if( ImGui::MenuItem( Label, "", Selected ) )
	{
		*Selected = true;

		return true;
	}

	return false;
}

static float Zoom = 1.0f;
static float ZoomTarget = 1.0f;
static ImVec2 Drag = ImVec2();
static class CTexture* PreviewTexture = nullptr;
static std::string PreviewName;
static float PreviousZoomWheel = -1.0f;
static bool TiledPreview = false;

std::unordered_map<std::string, CTexture*> Thumbnails;
CTexture* GetThumbnail( const std::string& Name )
{
	const auto Iterator = Thumbnails.find( Name );
	if( Iterator != Thumbnails.end() )
	{
		return Iterator->second;
	}

	return nullptr;
}

std::list<CModelPass> GeneratorList;
CTexture* GenerateThumbnail( const std::string& Name, CMesh* Mesh, MaterialAsset* Material, const float FieldOfView )
{
	RenderTextureConfiguration Configuration;
	Configuration.Width = 64;
	Configuration.Height = 64;
	Configuration.Format = EImageFormat::RGBA8;
	Configuration.Samples = 4;

	CRenderTexture* ThumbnailTexture = nullptr;

	const auto Iterator = Thumbnails.find( Name );
	if( Iterator == Thumbnails.end() )
	{
		ThumbnailTexture = new CRenderTexture( "ModelThumbnail", Configuration );
		Thumbnails.insert_or_assign( Name, ThumbnailTexture );
	}
	else
	{
		ThumbnailTexture = dynamic_cast<CRenderTexture*>( Iterator->second );
	}

	GeneratorList.emplace_back();
	auto& ModelPass = GeneratorList.back();
	ModelPass.Reset();
	ModelPass.Target = ThumbnailTexture;
	ModelPass.Model.SetMesh( Mesh );
	ModelPass.FieldOfView = FieldOfView;

	if( Material )
	{
		Material->Material.Apply( &ModelPass.Model );
	}

	auto& Window = CWindow::Get();
	CRenderer& Renderer = Window.GetRenderer();
	Renderer.AddRenderPass( &ModelPass, RenderPassLocation::Translucent );
	
	return GetThumbnail( Name );
}

void ShowTexture( CTexture* Texture )
{
	Zoom = Math::Lerp( Zoom, ZoomTarget, 0.2f );

	if( PreviewMesh )
	{
		if( !PreviewRenderTexture )
		{
			RenderTextureConfiguration Configuration;
			Configuration.Width = 512;
			Configuration.Height = 512;
			Configuration.Format = EImageFormat::RGBA8;
			
			PreviewRenderTexture = new CRenderTexture( "ModelPreviewTexture", Configuration );
		}
		
		auto& Window = CWindow::Get();
		CRenderer& Renderer = Window.GetRenderer();

		static CModelPass ModelPass;
		ModelPass.Reset();
		ModelPass.Target = PreviewRenderTexture;
		ModelPass.Model.SetMesh( PreviewMesh );

		if( PreviewMaterial )
		{
			PreviewMaterial->Material.Apply( &ModelPass.Model );
		}

		PreviewTexture = PreviewRenderTexture;
		Renderer.AddRenderPass( &ModelPass, RenderPassLocation::PreScene );
	}

	if( !Texture )
		return;
	
	auto ImageSize = ImVec2( Texture->GetWidth(), Texture->GetHeight() );

	if( ImageSize.x < 0.01f || ImageSize.y < 0.01f )
		return;
	
	auto ImageRatio = ImageSize.y / ImageSize.x;

	auto ContentOffset = ImGui::GetContentRegionAvail();

	auto MinimumSize = std::min( std::min( ImageSize.x, ImageSize.y ), std::min( ContentOffset.x, ContentOffset.y ) ) * ZoomTarget;

	ImageSize.x = MinimumSize;
	ImageSize.y = MinimumSize * ImageRatio;

	ContentOffset.x /= 2;
	ContentOffset.y /= 2;

	auto HalfSize = ImageSize;
	HalfSize.x /= 2;
	HalfSize.y /= 2;

	auto ContentPosition = ContentOffset;
	ContentPosition.x -= HalfSize.x;
	ContentPosition.y -= HalfSize.y;

	ContentPosition.x += Drag.x;
	ContentPosition.y += Drag.y;

	ImGui::SetCursorPos( ContentPosition );

	const auto CursorPosition = ImGui::GetCursorScreenPos();

	auto* TextureID = reinterpret_cast<ImTextureID>( Texture->GetHandle() );
	const float TileScale = TiledPreview ? 2.0f : 1.0f;
	ImGui::Image( TextureID, ImageSize, ImVec2( 0.0f, TileScale ), ImVec2( TileScale, 0.0f ) );
	ImGui::SetCursorPos( ContentPosition );
	ImGui::InvisibleButton( "PreviewPanel", ImageSize );

	if( ImGui::IsWindowHovered() )
	{
		float TextureX = ( ( ImGui::GetMousePos().x - CursorPosition.x ) / ImageSize.x ) * Texture->GetWidth();
		float TextureY = ( ( ImGui::GetMousePos().y - CursorPosition.y ) / ImageSize.y ) * Texture->GetHeight();

		if( TiledPreview )
		{
			TextureX = std::fmod( TextureX * 2.0f, Texture->GetWidth() );
			TextureY = std::fmod( TextureY * 2.0f, Texture->GetHeight() );
		}

		ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.8f );
		ImGui::BeginTooltip();
		ImGui::PopStyleVar();

		ImGui::Text( "X: %i Y: %i", (int) TextureX, (int) TextureY );
		ImGui::Text( "%ix%i", Texture->GetWidth(), Texture->GetHeight() );
		ImGui::Text( "%s", CAssets::Get().GetReadableImageFormat( Texture->GetImageFormat() ).c_str() );
		ImGui::EndTooltip();

		auto Delta = ImGui::GetIO().MouseWheel - PreviousZoomWheel;
		if( Delta > 0.1f )
		{
			ZoomTarget *= 1.2f;

			Drag.x *= 1.2f;
			Drag.y *= 1.2f;
		}
		else if( Delta < -0.1f )
		{
			ZoomTarget *= 0.8f;

			Drag.x *= 0.8f;
			Drag.y *= 0.8f;
		}

		ZoomTarget = std::max( ZoomTarget, 0.25f );
	}

	if( ImGui::IsItemActive() )
	{
		auto Delta = ImGui::GetMouseDragDelta( 0, 0.0f );
		Drag.x += Delta.x;
		Drag.y += Delta.y;
		ImGui::ResetMouseDragDelta( 0 );
	}

	ImGui::SetCursorPos( ImVec2( 0, 0 ) );

	ImGui::Text( "Zoom %.2f", ZoomTarget );

	if( ImGui::Button( "0.5x" ) )
	{
		ZoomTarget = 0.5f;
	}

	if( ImGui::Button( "1x" ) )
	{
		ZoomTarget = 1.0f;
	}

	if( ImGui::Button( "2x" ) )
	{
		ZoomTarget = 2.0f;
	}

	if( ImGui::Button( "Center" ) )
	{
		Drag = ImVec2();
	}

	if( ImGui::Button( TiledPreview ? "Single" : "Tile") )
	{
		TiledPreview = !TiledPreview;
	}
}

CTexture* UIFileSpeaker = nullptr; // Textures/UI/round_volume_up_black_48.png
CTexture* UIFileGeneric = nullptr; // Textures/UI/twotone_radio_button_unchecked_black_48.png

CTexture* CreateUIIcon( const char* Name, const char* Location )
{
	return CAssets::Get().CreateNamedTexture( Name, Location, EFilteringMode::Trilinear, EImageFormat::RGBA8 );
}

bool IconsCreated = false;
void CreateIcons()
{
	if( IconsCreated )
		return;
	
	UIFileSpeaker = CreateUIIcon( "ui_icon_speaker", "Textures/UI/round_volume_up_black_48.png" );
	UIFileGeneric = CreateUIIcon( "ui_icon_generic", "Textures/UI/twotone_radio_button_unchecked_black_48.png" );

	IconsCreated = true;
}

static bool ShowPreview = false;
static bool ShowTextures = true;
static bool ShowSounds = true;
static bool ShowMeshes = true;
static bool ShowShaders = true;
static bool ShowSequences = true;
static char FilterText[512] = {};

bool MatchFilter( const char* Name )
{
	return std::strstr( Name, FilterText ) != nullptr;
}

void ContentBrowserUI()
{
	CreateIcons();

	auto ContentHeight = ImGui::GetContentRegionAvail().y;
	if( ShowPreview )
		ContentHeight *= 0.5f;

	ImGui::BeginChild( "Content Panel", ImVec2( 0, ContentHeight ) );
	const int Columns = std::max( static_cast<int>( ImGui::GetWindowWidth() / 66 ), 1 );
	ImGui::Columns( Columns );

	const auto FilterLength = std::strlen( FilterText );
	const bool ValidFilter = FilterLength > 0;

	auto& Assets = CAssets::Get();
	if( ShowTextures )
	{
		for( const auto& Pair : Assets.Textures.Get() )
		{
			if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
				continue;
			
			auto* Texture = Assets.Textures.Get( Pair.second );
			if( !Texture )
				continue;

			auto ImageSize = ImVec2( 64, 64 );

			auto* TextureID = reinterpret_cast<ImTextureID>( Texture->GetHandle() );
			if( ImGui::ImageButton( TextureID, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1, ImVec4( 0, 0, 0, 0 ) ) )
			{
				PreviewName = Pair.first;
				PreviewTexture = Texture;
				PreviewMesh = nullptr;
				PreviewMaterial = nullptr;
				ShowPreview = true;
			}

			if( ImGui::IsItemHovered() )
			{
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.8f );
				ImGui::BeginTooltip();
				ImGui::PopStyleVar();

				ImGui::Text( "%s", Pair.first.c_str() );
				ImGui::Text( "%s", Texture->GetLocation().c_str() );
				ImGui::Text( "%ix%i", Texture->GetWidth(), Texture->GetHeight() );
				ImGui::Text( "%s", CAssets::GetReadableImageFormat( Texture->GetImageFormat() ).c_str() );
				ImGui::EndTooltip();
			}

			ImGui::NextColumn();
		}
	}

	if( ShowMeshes )
	{
		for( const auto& Pair : Assets.Meshes.Get() )
		{
			if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
				continue;

			auto* Mesh = Assets.Meshes.Get( Pair.second );
			if( !Mesh )
				continue;

			auto ImageSize = ImVec2( 64, 64 );

			bool ShouldGenerateThumbnail = false;
			CTexture* Thumbnail = GetThumbnail( Pair.first );
			if( !Thumbnail )
			{
				ShouldGenerateThumbnail = true;
				// GenerateThumbnail( Pair.first, Mesh );
			}

			if( !Thumbnail )
			{
				Thumbnail = UIFileGeneric;
			}
			
			auto* TextureID = reinterpret_cast<ImTextureID>( Thumbnail->GetHandle() );
			const ImVec4 Background = ImVec4( 0, 0, 0, 0 );
			const ImVec4 Tint = ShouldGenerateThumbnail ? ImVec4( 0, 1.0f, 0, 1.0f ) : ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

			ImGui::PushID( Pair.first.c_str() );
			if( ImGui::ImageButton(
				TextureID, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ), 0,
				Background,
				Tint ) )
			{
				PreviewName = Pair.first;
				PreviewTexture = nullptr;
				PreviewMesh = Mesh;
				PreviewMaterial = nullptr;
				ShowPreview = true;
			}

			if( ImGui::IsItemHovered() )
			{
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.8f );
				ImGui::BeginTooltip();
				ImGui::PopStyleVar();
				ImGui::Text( "%s", Pair.first.c_str() );
				ImGui::Text( "%s", Mesh->GetLocation().c_str() );

				ImGui::Separator();

				const auto& Bounds = Mesh->GetBounds();
				auto Size = ( Bounds.Maximum - Bounds.Minimum );
				const auto Meters = Size.X > 1.0f && Size.Y > 1.0f && Size.Z > 1.0f;
				if( !Meters )
				{
					Size *= 100.0f;
					ImGui::Text( "%.0fcm %.0fcm %.0fcm", Size.X, Size.Y, Size.Z );
				}
				else
				{
					ImGui::Text( "%.1fm %.1fm %.1fm", Size.X, Size.Y, Size.Z );
				}
				ImGui::Separator();

				// ImGui::Text( "Vertices: %llu", Mesh->GetVertexData().Vertices );

				const auto& AnimationSet = Mesh->GetAnimationSet();
				if( !AnimationSet.Skeleton.Bones.empty() )
					ImGui::Text( "Bones: %llu", Mesh->GetAnimationSet().Skeleton.Bones.size() );
				if( !AnimationSet.Skeleton.Animations.empty() )
				{
					ImGui::Text( "Animations: %llu", Mesh->GetAnimationSet().Skeleton.Animations.size() );

					ImGui::Separator();
					ImGui::Columns( 2 );
					for( auto& Animation : AnimationSet.Set )
					{
						ImGui::Text( "%s", Animation.first.c_str() );
						ImGui::NextColumn();

						ImGui::Text( "(%s)", Animation.second.c_str() );
						ImGui::NextColumn();
					}
					ImGui::Columns( 1 );
				}

				ImGui::EndTooltip();

				GenerateThumbnail( Pair.first, Mesh );
			}

			if( ImGui::IsItemVisible() && ShouldGenerateThumbnail )
			{
				GenerateThumbnail( Pair.first, Mesh );
			}

			ImGui::PopID();

			ImGui::NextColumn();
		}
	}

	auto* PrimitiveCube = Assets.Meshes.Find( "cube" );
	for( const auto& Pair : Assets.Assets.Get() )
	{
		if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
			continue;

		auto* Asset = Assets.Assets.Get( Pair.second );
		if( !Asset )
			continue;

		bool ShouldGenerateThumbnail = false;
		CTexture* Thumbnail = nullptr;

		MaterialAsset* Material = nullptr;
		if( Material = dynamic_cast<MaterialAsset*>( Asset ) )
		{
			Thumbnail = GetThumbnail( Pair.first );
			if( !Thumbnail )
			{
				ShouldGenerateThumbnail = true;
				GenerateThumbnail( Pair.first, PrimitiveCube, Material );
			}

			if( !Thumbnail )
			{
				Thumbnail = UIFileGeneric;
			}
		}
		else
		{
			Thumbnail = UIFileGeneric;
		}

		const bool IconOnly = Thumbnail == UIFileGeneric;
		auto ImageSize = ImVec2( 64, 64 );
		ImTextureID TextureID = reinterpret_cast<ImTextureID>( Thumbnail->GetHandle() );
		const ImVec4 Background = ImVec4( 0, 0, 0, 0 );
		const ImVec4 Tint = Thumbnail == UIFileGeneric ? ImVec4( 0.25f, 0.5f, 1.0f, 1.0f ) : ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

		ImGui::PushID( Pair.first.c_str() );
		if( ImGui::ImageButton(
			TextureID, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1,
			Background,
			Tint ) )
		{
			PreviewName = Pair.first;
			PreviewTexture = IconOnly ? nullptr : Thumbnail;
			PreviewMesh = IconOnly ? nullptr : PrimitiveCube;
			PreviewMaterial = Material;
			ShowPreview = true;
		}

		if( ImGui::IsItemHovered() )
		{
			ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.8f );
			ImGui::BeginTooltip();
			ImGui::PopStyleVar();

			ImGui::Text( "%s", Pair.first.c_str() );
			ImGui::Text( "Type: %s", Asset->GetType().c_str() );

			ImGui::EndTooltip();

			GenerateThumbnail( Pair.first, PrimitiveCube, Material );
		}

		ImGui::PopID();

		ImGui::NextColumn();
	}

	if( ShowSounds )
	{
		for( const auto& Pair : Assets.Sounds.Get() )
		{
			if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
				continue;

			auto* Sound = Assets.Sounds.Get( Pair.second );
			if( !Sound )
				continue;

			auto ImageSize = ImVec2( 64, 64 );

			auto* TextureID = reinterpret_cast<ImTextureID>( UIFileSpeaker->GetHandle() );
			const bool Playing = Sound->Playing();
			const ImVec4 Background = ImVec4( 0, 0, 0, 0 );
			const ImVec4 Tint = Playing ? ImVec4( 1.0f, 0, 0, 1.0f ) : ImVec4( 0.65f, 0.1f, 0.1f, 1.0f );

			ImGui::PushID( Pair.first.c_str() );
			if( ImGui::ImageButton(
				TextureID, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1,
				Background,
				Tint ) )
			{
				PreviewName = Pair.first;
				PreviewTexture = UIFileSpeaker;
				PreviewMesh = nullptr;
				ShowPreview = true;

				if( !Playing )
				{
					auto Information = Spatial::CreateUI();

					static uint32_t BusIndex = Bus::Auxilery3;
					if( BusIndex == Bus::Maximum )
						BusIndex = Bus::Auxilery3;

					Information.Bus = Bus::Type( BusIndex++ );

					Sound->Start( Information );
				}
				else
				{
					Sound->Stop();
				}
			}

			if( ImGui::IsItemHovered() )
			{
				ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.8f );
				ImGui::BeginTooltip();
				ImGui::PopStyleVar();

				ImGui::Text( "%s", Pair.first.c_str() );
				ImGui::Text( "Type: %s", Sound->GetSoundType() == ESoundType::Stream ? "Stream" : "Memory" );

				if( Sound->GetSoundType() == ESoundType::Memory )
				{
					ImGui::Text( "Sounds: %llu", Sound->GetSoundBufferHandles().size() );
				}

				ImGui::EndTooltip();
			}

			ImGui::PopID();

			ImGui::NextColumn();
		}
	}

	ImGui::Columns( 1 );
	ImGui::EndChild();

	if( !ShowPreview )
		return;

	ImGui::BeginChild( "Texture Preview", ImVec2( 0, 0 ), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
	ShowTexture( PreviewTexture );
	ImGui::EndChild();
}

static bool ShowNewAssetPopup = false;
static std::string NewAssetType = "Sequence";
static std::string NewAssetName = "NewAsset";

AssetNewPopup AssetPopup;

void NewAssetUI()
{
	DisplayAssetWindow( AssetPopup );
	return;
	if( ShowNewAssetPopup )
	{
		if( ImGui::BeginPopupContextItem( "New Asset", 0 ) )
		{
			ImGui::Text( "New Asset" );
			ImGui::Separator();

			if( ImGui::BeginCombo( "Type", NewAssetType.c_str() ) )
			{
				if( ImGui::Selectable( "Sequence" ) )
				{
					NewAssetType = "Sequence";
				}

				if( ImGui::Selectable( "Sound" ) )
				{
					NewAssetType = "Sound";
				}

				ImGui::Separator();

				ImGui::EndCombo();
			}

			const bool SequenceAsset = NewAssetType == "Sequence";
			const bool SoundAsset = NewAssetType == "Sound";
			
			static char AssetName[128];
			ImGui::InputText( "Name", AssetName, 128 );
			ImGui::SetItemDefaultFocus();

			static char AssetLocation[512];
			static bool AudioStream;
			if( SoundAsset )
			{
				ImGui::InputText( "Location", AssetLocation, 512 );
				ImGui::SameLine();
				if( ImGui::Button( "..." ) )
				{
					DialogFormats Formats;
					Formats.insert_or_assign( L"Audio", L"*.ogg;*.flac;*.wav;" );
					const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );					
					strcpy_s( AssetLocation, Path.c_str() );
				}
				
				ImGui::Checkbox( "Stream", &AudioStream );
			}

			if( ImGui::Button( "Create" ) )
			{
				auto& Assets = CAssets::Get();
				if( SequenceAsset )
				{
					NewAssetName = AssetName;
					if( NewAssetName.length() > 0 )
					{
						const std::string AssetPath = "Sequences/" + NewAssetName + ".lsq";
						auto* Sequence = Assets.CreateNamedSequence( AssetName, AssetPath.c_str() );
						Sequence->Draw();
					}
				}
				else if( SoundAsset )
				{
					NewAssetName = AssetName;
					if( NewAssetName.length() > 0 )
					{
						if( AudioStream )
						{
							Assets.CreateNamedStream( AssetName, AssetLocation );
						}
						else
						{
							Assets.CreateNamedSound( AssetName, AssetLocation );
						}
					}
				}

				ShowNewAssetPopup = false;
			}

			ImGui::EndPopup();
		}

		if( !ShowNewAssetPopup )
		{
			ImGui::CloseCurrentPopup();
			NewAssetName = "";
		}
	}
}

static bool ShowAssets = false;
bool* DisplayAssets()
{
	return &ShowAssets;
}

void ReloadMesh( const std::string& Name, const std::string& Location )
{
	auto& Assets = CAssets::Get();
	Timer LoadTimer;

	LoadTimer.Start();
	auto* Mesh = Assets.CreateNamedMesh( Name.c_str(), Location.c_str(), true );
	LoadTimer.Stop();

	const size_t Triangles = Mesh->GetVertexBufferData().IndexCount / 3;

	Log::Event( "Re-import: %ims %i triangles\n", LoadTimer.GetElapsedTimeMilliseconds(), Triangles );
}

void ReloadAllMeshes()
{
	auto& Assets = CAssets::Get();
	
	for( const auto& Pair : Assets.Meshes.Get() )
	{
		auto* Mesh = Assets.Meshes.Get( Pair.second );
		if( !Mesh )
			continue;

		ReloadMesh( Pair.first, Mesh->GetLocation() );
	}
}

void AssetUI()
{
	auto& Assets = CAssets::Get();

	if( ShowAssets )
	{
		if( ImGui::Begin( "Assets", &ShowAssets, ImVec2( 1000.0f, 700.0f ) ) )
		{
			if( ImGui::Button( "New##NewAssetPopup" ) )
			{
				// ShowNewAssetPopup = true;
				OpenAssetWindow( AssetPopup.Window );
			}

			ImGui::SameLine();

			NewAssetUI();

			if( ImGui::Button( "Reload Meshes##MeshReload" ) )
			{
				ReloadAllMeshes();
			}

			ImGui::SameLine();

			ImGui::SameLine(); ImGui::Checkbox( "Toggle Preview", &ShowPreview );
			ImGui::SameLine(); ImGui::Checkbox( "Textures", &ShowTextures );
			ImGui::SameLine(); ImGui::Checkbox( "Sounds", &ShowSounds );
			ImGui::SameLine(); ImGui::Checkbox( "Meshes", &ShowMeshes );
			ImGui::SameLine(); ImGui::Checkbox( "Shaders", &ShowShaders );
			ImGui::SameLine(); ImGui::Checkbox( "Sequences", &ShowSequences );

			ImGui::PushItemWidth( -1.0f );
			ImGui::InputText( "##AssetFilter", FilterText, 512 );
			ImGui::PopItemWidth();

			ImGui::BeginChild( "Asset List", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.33f, ImGui::GetContentRegionAvail().y ) );
			ImGui::Columns( 2 );

			ImGui::Separator();
			ImGui::Text( "Name" ); ImGui::NextColumn();
			ImGui::Text( "Type" ); ImGui::NextColumn();

			ImGui::Separator();
			ImGui::Separator();

			const bool InclusiveFilter = ShowTextures && ShowSounds && ShowMeshes && ShowShaders && ShowSequences;

			const auto FilterLength = std::strlen( FilterText );
			const bool ValidFilter = FilterLength > 0;

			auto* World = CWorld::GetPrimaryWorld();
			if( InclusiveFilter && World )
			{
				auto& Levels = World->GetLevels();
				for( auto& Level : Levels )
				{
					if( ValidFilter && !MatchFilter( Level.GetName().c_str() ) )
						continue;

					if( ImGui::Selectable( Level.GetName().c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
					{
						Level.Reload();
					}
					ImGui::NextColumn();
					ImGui::Text( "Level" ); ImGui::NextColumn();
					ImGui::Separator();
				}
			}

			ImGui::Separator();
			ImGui::Separator();

			if( InclusiveFilter )
			{
				for( const auto& Pair : Assets.Assets.Get() )
				{
					if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
						continue;

					auto* Asset = Assets.Assets.Get( Pair.second );
					if( !Asset )
						continue;

					if( ImGui::Selectable( ( Pair.first + "##Custom" ).c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
					{
						Asset->Reload();
					}
					ImGui::NextColumn();

					ImGui::Text( Asset->GetType().c_str() ); ImGui::NextColumn();
					ImGui::Separator();
				}
			}

			ImGui::Separator();
			ImGui::Separator();

			if( ShowMeshes )
			{
				const auto& Meshes = Assets.Meshes.Get();
				for( const auto& Pair : Meshes )
				{
					if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
						continue;

					auto* Mesh = Assets.Meshes.Get( Pair.second );
					if( !Mesh )
						continue;

					if( ImGui::Selectable( ( Pair.first + "##Mesh" ).c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
					{
						if( Assets.Meshes.Find( Pair.first ) )
						{
							ReloadMesh( Pair.first, Mesh->GetLocation() );
						}
					}
					ImGui::NextColumn();
					ImGui::Text( "Mesh" ); ImGui::NextColumn();
					ImGui::Separator();
				}
			}

			ImGui::Separator();

			if( ShowShaders )
			{
				for( const auto& Pair : Assets.Shaders.Get() )
				{
					if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
						continue;

					auto* Shader = Assets.Shaders.Get( Pair.second );
					if( !Shader )
						continue;

					// ImGui::Text( Pair.first.c_str() ); ImGui::NextColumn();
					std::string Label = Pair.first;
					if( Shader->AutoReload() )
					{
						Label += "( Auto Reload )";
					}

					Label += "##Shader";

					if( ImGui::Selectable( Label.c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
					{
						Shader->Reload();
					}

					if( ImGui::IsItemClicked( 1 ) )
					{
						Shader->AutoReload( !Shader->AutoReload() );
					}

					if( ImGui::IsItemHovered() )
					{
						ImGui::PushStyleVar( ImGuiStyleVar_Alpha, 0.8f );
						ImGui::BeginTooltip();
						ImGui::PopStyleVar();

						ImGui::Text( "Left click to reload.\nRight click to enable auto-reload." );
						ImGui::EndTooltip();
					}

					ImGui::NextColumn();

					ImGui::Text( "Shader" ); ImGui::NextColumn();
					ImGui::Separator();
				}
			}

			ImGui::Separator();

			if( ShowTextures )
			{
				for( const auto& Pair : Assets.Textures.Get() )
				{
					if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
						continue;

					auto* Texture = Assets.Textures.Get( Pair.second );
					if( !Texture )
						continue;

					// ImGui::Text( Pair.first.c_str() ); ImGui::NextColumn();
					if( ImGui::Selectable( ( Pair.first + "##Texture" ).c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
					{
						PreviewTexture = Texture;
					}
					ImGui::NextColumn();

					ImGui::Text( "Texture" ); ImGui::NextColumn();
					ImGui::Separator();
				}
			}

			ImGui::Separator();

			if( ShowSounds )
			{
				for( const auto& Pair : Assets.Sounds.Get() )
				{
					if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
						continue;

					auto* Sound = Assets.Sounds.Get( Pair.second );
					if( !Sound )
						continue;

					if( ImGui::Selectable( ( Pair.first + "##Audio" ).c_str(), Sound->Playing(), ImGuiSelectableFlags_SpanAllColumns ) )
					{
						if( Sound->Playing() )
						{
							Sound->Stop();
						}
						else
						{
							Sound->Start();
						}
					}
					ImGui::NextColumn();

					// ImGui::Text( Pair.first.c_str() ); ImGui::NextColumn();
					if( Sound->GetSoundType() == ESoundType::Stream )
					{
						ImGui::Text( "Stream" ); ImGui::NextColumn();
					}
					else
					{
						ImGui::Text( "Sound" ); ImGui::NextColumn();
					}

					ImGui::Separator();
				}
			}

			if( ShowSequences )
			{
				for( const auto& Pair : Assets.Sequences.Get() )
				{
					if( ValidFilter && !MatchFilter( Pair.first.c_str() ) )
						continue;

					auto* Sequence = Assets.Sequences.Get( Pair.second );
					if( !Sequence )
						continue;

					if( ImGui::Selectable( ( Pair.first + "##Sequence" ).c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
					{
						Sequence->Draw();
					}
					ImGui::NextColumn();

					ImGui::Text( "Sequence" ); ImGui::NextColumn();
					ImGui::Separator();
				}
			}

			ImGui::Separator();
			ImGui::Columns( 1 );
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::BeginChild( "Texture List", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.66f, 0 ) );
			ContentBrowserUI();
			ImGui::EndChild();
		}

		ImGui::End();
	}
}

static bool ShowStrings = false;
bool* DisplayStrings()
{
	return &ShowStrings;
}

void StringUI()
{
	if( ShowStrings )
	{
		if( ImGui::Begin( "Strings", &ShowStrings, ImVec2( 500.0f, 700.0f ) ) )
		{
			ImGui::Columns( 2 );

			ImGui::Separator();
			ImGui::Text( "String" ); ImGui::NextColumn();
			ImGui::Text( "Index" ); ImGui::NextColumn();

			ImGui::Separator();
			ImGui::Separator();

			for( const auto& Pair : NamePool::Get().Pool() )
			{
				ImGui::Text( Pair.first.c_str() ); ImGui::NextColumn();
				ImGui::Text( "%i", Pair.second ); ImGui::NextColumn();
				ImGui::Separator();
			}

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Columns( 1 );
		}

		ImGui::End();
	}
}

void SetPreviewTexture( CTexture* Texture )
{
	PreviewTexture = Texture;
}

void SetMouseWheel( const float& Wheel )
{
	PreviousZoomWheel = Wheel;
}

static bool ShowMixer = false;
bool* DisplayMixer()
{
	return &ShowMixer;
}

static bool ShowAudioPlayer = false;
bool* DisplayAudioPlayer()
{
	return &ShowAudioPlayer;
}

static bool ShowShaderToy = false;
bool* DisplayShaderToy()
{
	return &ShowShaderToy;
}

static bool ShowTagList = false;
bool* DisplayTagList()
{
	return &ShowTagList;
}

static bool ShowColorManagement = false;
bool* DisplayColorManagement()
{
	return &ShowColorManagement;
}

static bool ShowCameraInformation = false;
bool* DisplayCameraInformation()
{
	return &ShowCameraInformation;
}

static auto TranslateBus = Translate<std::string, Bus::Type>( {
		{"Master", Bus::Master},
		{"SFX", Bus::SFX},
		{"Music", Bus::Music},
		{"UI", Bus::UI},
		{"Dialogue", Bus::Dialogue},
		{"AUX3", Bus::Auxilery3},
		{"AUX4", Bus::Auxilery4},
		{"AUX5", Bus::Auxilery5},
		{"AUX6", Bus::Auxilery6},
		{"AUX7", Bus::Auxilery7},
		{"AUX8", Bus::Auxilery8}
	}
);

std::vector<std::string> EffectNames = {
	"Reverb",
	"Echo",
	"Limiter"
};

//float AmplitudeTodB( const float& Amplitude )
//{
//	static const float Reference = std::powf( 10.0f, -5.0f );
//	const float X = std::fabs( Amplitude );
//	return 20 * std::log10f( X / Reference );
//}

float AmplitudeTodB( const float& Amplitude )
{
	return 20 * std::log10f( Amplitude );
}

float dBToAmplitude( const float& Decibels )
{
	return std::powf( 10, Decibels / 20 );
}

struct BusInformation
{
	Bus::Volume Volume;
	float PeakLeft = 0.0f;
	float PeakRight = 0.0f;
};

static bool HoveringBus = false;
std::unordered_map<std::string, BusInformation> BusEntry;
static int BusID = 0;
void DrawBus( const std::string& Name, const Bus::Type& Bus )
{
	ImGui::BeginChild( Name.c_str(), ImVec2( 50.0f, 200.0f ), true );
	if( ImGui::IsWindowHovered() )
	{
		HoveringBus = true;
	}
	
	const auto Volume = SoLoudSound::GetBusOutput( Bus );
	if( BusEntry.find( Name ) == BusEntry.end() )
	{
		BusInformation Information;
		Information.Volume = Volume;
		Information.PeakLeft = Volume.Left;
		Information.PeakRight = Volume.Right;
		BusEntry.insert_or_assign( Name, Information );
	}
	
	const float MaximumVolume = SoLoudSound::Volume( Bus );

	const auto PreviousFrame = BusEntry[Name];
	const auto PreviousVolume = PreviousFrame.Volume;

	const float BlendFactor = Math::Saturate( GameLayersInstance->GetFrameTime() * 20.0f );
	Bus::Volume NewVolume;
	NewVolume.Left = Math::Lerp( PreviousVolume.Left, Volume.Left, BlendFactor );
	NewVolume.Right = Math::Lerp( PreviousVolume.Right, Volume.Right, BlendFactor );

	BusInformation Information;
	Information.Volume = NewVolume;
	Information.PeakLeft = Math::Max( NewVolume.Left * MaximumVolume, PreviousFrame.PeakLeft * 0.99f );
	Information.PeakRight = Math::Max( NewVolume.Right * MaximumVolume, PreviousFrame.PeakRight * 0.99f );
	BusEntry.insert_or_assign( Name, Information );

	const float DecibelLeft = AmplitudeTodB( NewVolume.Left );
	const float DecibelRight = AmplitudeTodB( NewVolume.Right );
	const float ScaleLeft = NewVolume.Left;
	const float ScaleRight = NewVolume.Right;

	auto* DrawList = ImGui::GetWindowDrawList();
	const auto Position = ImGui::GetCursorScreenPos();
	const auto ButtonText = Name + "##Volume";

	const auto BusStyle = ImGui::GetStyleColorVec4( ImGuiCol_PlotHistogram );

	ImColor BaseColor = ImColor(
		BusStyle.x,
		BusStyle.y,
		BusStyle.z,
		0.1f );

	auto LeftColor = ImColor( 
		BusStyle.x,
		BusStyle.y,
		BusStyle.z,
		0.85f );
	auto RightColor = ImColor( 
		BusStyle.x,
		BusStyle.y,
		BusStyle.z,
		0.85f );
	
	if( DecibelLeft > -0.5f )
	{
		LeftColor = ImColor( 2.0f, 0.0f, 0.0f, 0.75f );
		BaseColor = LeftColor;
	}

	if( DecibelRight > -0.5f )
	{
		RightColor = ImColor( 2.0f, 0.0f, 0.0f, 0.75f );
		BaseColor = RightColor;
	}

	const float VerticalMarginBottom = 190.0f;
	const float VerticalMarginTop = 20.0f;
	const float VerticalPositionBottom = Position.y + VerticalMarginBottom;
	const float VerticalPositionTop = Position.y + VerticalMarginTop;

	const float HorizontalMargin = 20.0f;
	const float HorizontalPosition = Position.x + HorizontalMargin;
	const float HorizontalPositionDouble = Position.x + HorizontalMargin * 2.0f;

	const float LeftMaximum = Math::Saturate( 1.0f - ScaleLeft * MaximumVolume ) * 170.0f;
	const float RightMaximum = Math::Saturate( 1.0f - ScaleRight * MaximumVolume ) * 170.0f;
	
	DrawList->AddRectFilledMultiColor( 
		ImVec2( Position.x, VerticalPositionBottom ), ImVec2( HorizontalPosition, VerticalPositionTop + LeftMaximum ),
		BaseColor, BaseColor, LeftColor, LeftColor
	);
	
	DrawList->AddRectFilledMultiColor( 
		ImVec2( HorizontalPosition, VerticalPositionBottom ), ImVec2( HorizontalPositionDouble, VerticalPositionTop + RightMaximum ),
		BaseColor, BaseColor, RightColor, RightColor
	);

	if( Bus != Bus::Master )
	{
		BaseColor = ImColor( 0.0f, 0.0f, 0.0f, 0.25f );
		LeftColor = ImColor( 0.0f, 0.0f, 0.0f, 0.25f );
		RightColor = ImColor( 0.0f, 0.0f, 0.0f, 0.25f );

		DrawList->AddRectFilledMultiColor(
			ImVec2( Position.x, VerticalPositionTop + LeftMaximum ), ImVec2( HorizontalPosition, VerticalPositionTop + Math::Saturate( 1.0f - ScaleLeft ) * 170.0f ),
			BaseColor, BaseColor, LeftColor, LeftColor
		);

		DrawList->AddRectFilledMultiColor(
			ImVec2( HorizontalPosition, VerticalPositionTop + RightMaximum ), ImVec2( HorizontalPositionDouble, VerticalPositionTop + Math::Saturate( 1.0f - ScaleRight ) * 170.0f ),
			BaseColor, BaseColor, RightColor, RightColor
		);
	}

	const int Lines = 10;
	for( int LineIndex = 0; LineIndex < Lines; LineIndex++ )
	{
		float Magnitude = StaticCast<float>( LineIndex ) / StaticCast<float>( Lines );
		Magnitude = std::powf( Magnitude, 0.5f );
		DrawList->AddLine( 
			ImVec2( Position.x, VerticalPositionTop + 170.0f * Magnitude ),
			ImVec2( Position.x + 40.0f, Position.y + 20.0f + 170.0f * Magnitude ), 
			ImColor( 0.0f, 0.0f, 0.0f, 0.125f )
		);

		const float Decibels = AmplitudeTodB( 1.0f - Magnitude );
		const auto DecibelString = std::to_string( StaticCast<int>( Decibels ) );
		DrawList->AddText(
			ImVec2( Position.x, VerticalPositionTop + 170.0f * Magnitude ),
			ImColor( 0.0f, 0.0f, 0.0f, 0.25f ),
			DecibelString.c_str()
		);
	}

	DrawList->AddLine(
		ImVec2( Position.x, VerticalPositionTop + 170.0f * ( 1.0f - Information.PeakLeft ) ),
		ImVec2( HorizontalPosition, VerticalPositionTop + 170.0f * ( 1.0f - Information.PeakLeft ) ),
		ImColor( Information.PeakLeft, 0.0f, 2.0f - Information.PeakLeft * 2.0f, Information.PeakLeft ),
		3.0f
	);

	DrawList->AddLine(
		ImVec2( HorizontalPosition, VerticalPositionTop + 170.0f * ( 1.0f - Information.PeakRight ) ),
		ImVec2( HorizontalPositionDouble, VerticalPositionTop + 170.0f * ( 1.0f - Information.PeakRight ) ),
		ImColor( Information.PeakRight, 0.0f, 2.0f - Information.PeakRight * 2.0f, Information.PeakRight ),
		3.0f
	);
	
	ImGui::Text( "%s", Name.c_str() );

	ImGui::SetCursorScreenPos( ImVec2( Position.x + 10.0f, Position.y + 20.0f + ( 1.0f - MaximumVolume ) * 160.0f ) );
	const auto VolumeText = "##Adjust" + Name;

	ImGui::Button( VolumeText.c_str(), ImVec2( 10.0f, 10.0f ) );
	if( ImGui::IsItemHovered() || ( ImGui::IsWindowFocused() && ImGui::IsMouseDragging( 0, 0.0f ) ) )
	{
		const float Drag = ImGui::GetMouseDragDelta( 0, 0.0f ).y / 160.0f;
		if( Math::Abs( Drag ) > 0.001f )
		{
			SoLoudSound::Volume( Bus, Math::Saturate( MaximumVolume - Drag ) );
			ImGui::ResetMouseDragDelta();
		}
	}
	
	ImGui::EndChild();

	ImGui::SameLine();
	const auto NewPosition = ImGui::GetCursorScreenPos();

	ImGui::SetCursorScreenPos( ImVec2( Position.x + 10.0f, Position.y + 200.0f ) );

	const auto MuteText = "M##" + Name;
	bool Muted = MaximumVolume < 0.001f;
	if( ImGui::Checkbox( MuteText.c_str(), &Muted ) )
	{
		SoLoudSound::Volume( Bus, SoLoudSound::Volume( Bus ) > 0.001f ? 0.0f : 1.0f );
	}

	ImGui::SetCursorScreenPos( ImVec2( Position.x + 5.0f, Position.y + 225.0f ) );

	auto& Stack = SoLoudSound::GetBusStack( Bus );
	const auto& Effects = Stack.Get();

	const Effect* MarkedEffect = nullptr;

	const auto FXText = "##FX" + Name;
	ImGui::SetNextItemWidth( HorizontalMargin * 2.0f );
	if( ImGui::ListBoxHeader( FXText.c_str(), Effects.size(), 8 ) )
	{
		for( auto& Effect : Effects )
		{
			const std::string EffectName = Effect.Name.length() > 0 ? Effect.Name : "Unnamed Effect";
			ImGui::Selectable( EffectName.c_str(), Effect.Inactive == nullptr );

			if( ImGui::IsItemHovered() )
			{
				ImGui::BeginTooltip();
				ImGui::Text( "%s", EffectName.c_str() );
				ImGui::EndTooltip();
			}

			if( ImGui::IsItemClicked( 0 ) || ImGui::IsItemClicked( 1 ) )
			{
				MarkedEffect = &Effect;
			}
		}
		
		ImGui::ListBoxFooter();
	}

	if( MarkedEffect )
	{
		Stack.Toggle( MarkedEffect );
	}

	ImGui::SetCursorScreenPos( NewPosition );
}

void MixerUI()
{
	if( !ShowMixer )
		return;

	ImGuiWindowFlags Flags = ImGuiWindowFlags_AlwaysAutoResize;
	if( HoveringBus && ImGui::IsMouseDown( 0 ) )
	{
		Flags |= ImGuiWindowFlags_NoMove;
	}

	HoveringBus = false;
	if( ImGui::Begin( "Mixer", &ShowMixer, ImVec2( 1000.0f, 700.0f ), -1, Flags ) )
	{
		DrawBus( "Master", Bus::Master );
		DrawBus( "SFX", Bus::SFX );
		DrawBus( "Music", Bus::Music );
		DrawBus( "UI", Bus::UI );
		DrawBus( "Dialogue", Bus::Dialogue );
		DrawBus( "AUX3", Bus::Auxilery3 );
		DrawBus( "AUX4", Bus::Auxilery4 );
		DrawBus( "AUX5", Bus::Auxilery5 );
		DrawBus( "AUX6", Bus::Auxilery6 );
		DrawBus( "AUX7", Bus::Auxilery7 );
		DrawBus( "AUX8", Bus::Auxilery8 );
	}

	ImGui::End();
}

std::string AudioPlayerLocation;
SoundInstance AudioPlayerInstance;
Spatial AudioPlayerSpatial = Spatial::Create();
bool AudioPlayerLoop = false;

float PlayPosition = 0.0f;
float LastPlayPosition = 0.0f;
float ActualPosition = 0.0f;

bool AudioPlayerFirstOpen = true;
static CSound* AudioPlayerStream = nullptr;

void AudioPlayerUI()
{
	if( !ShowAudioPlayer )
		return;

	if( AudioPlayerFirstOpen )
	{
		// Set values we want the user to be able to modify later.
		AudioPlayerSpatial.Bus = Bus::Auxilery8;
		AudioPlayerSpatial.Volume = 25.0f;
		AudioPlayerFirstOpen = false;
	}

	const ImGuiWindowFlags Flags = 0;
	if( ImGui::Begin( "Audio Player", &ShowAudioPlayer, ImVec2( 1000.0f, 700.0f ), -1, Flags ) )
	{
		if( ImGui::Button( "..." ) )
		{
			DialogFormats Formats;
			Formats.insert_or_assign( L"Audio", L"*.ogg;*.flac;*.wav;" );

			const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
			if( Path.length() > 0 )
			{
				AudioPlayerLocation = Path;

				if( !AudioPlayerStream )
				{
					AudioPlayerStream = new CSound( ESoundType::Stream );
				}

				AudioPlayerStream->Clear( true );
				AudioPlayerStream->Load( Path.c_str() );

				PlayPosition = 0.0f;
				LastPlayPosition = 0.0f;
				ActualPosition = 0.0f;

				AudioPlayerInstance.Stop( 0.1f );
				AudioPlayerInstance = SoundInstance( AudioPlayerStream );
				AudioPlayerInstance.Start( AudioPlayerSpatial );
				AudioPlayerInstance.Loop( AudioPlayerLoop );
			}
		}

		ImGui::SameLine();
		
		ImGui::TextWrapped( "Playing \"%s\"", AudioPlayerLocation.c_str() );

		ImGui::Columns( 2 );

		if( ImGui::Button( "Play" ) )
		{
			AudioPlayerInstance.Stop( 0.1f );
			AudioPlayerInstance.Start( AudioPlayerSpatial );
			AudioPlayerInstance.Loop( AudioPlayerLoop );

			PlayPosition = 0.0f;
			LastPlayPosition = 0.0f;
			ActualPosition = 0.0f;
		}

		ImGui::SameLine();

		if( ImGui::Button( "Stop" ) )
		{
			AudioPlayerInstance.Stop( 0.1f );

			PlayPosition = 0.0f;
			LastPlayPosition = 0.0f;
			ActualPosition = 0.0f;
		}

		ImGui::SameLine();

		if( ImGui::Checkbox( "Loop", &AudioPlayerLoop ) )
		{
			AudioPlayerInstance.Loop( AudioPlayerLoop );
		}

		ImGui::NextColumn();

		if( ImGui::SliderFloat( "Volume", &AudioPlayerSpatial.Volume, 0.0f, 300.0f, "%.0f" ) )
		{
			AudioPlayerInstance.Volume( AudioPlayerSpatial.Volume );
			
		}

		if( ImGui::SliderFloat( "Rate", &AudioPlayerSpatial.Rate, 0.0f, 5.0f, "%.1f" ) )
		{
			AudioPlayerInstance.Rate( AudioPlayerSpatial.Rate );
		}

		const auto NewBus = ImGui::BusSelector( AudioPlayerSpatial.Bus );

		if( NewBus != AudioPlayerSpatial.Bus )
		{
			AudioPlayerSpatial.Bus = NewBus;
		}

		ImGui::Columns( 1 );

		const bool Playing = AudioPlayerInstance.Playing();

		ImGui::SetNextItemWidth( -1.0f );
		if( ImGui::SliderFloat( "##TimeBar", &ActualPosition, 0.0f, AudioPlayerInstance.Length(), "%.1f" ) )
		{
			if( Playing )
			{
				AudioPlayerInstance.Stop();
				AudioPlayerInstance.Start( AudioPlayerSpatial );
				AudioPlayerInstance.Loop( AudioPlayerLoop );
				AudioPlayerInstance.Offset( std::fmod( ActualPosition, AudioPlayerInstance.Length() ) / AudioPlayerSpatial.Rate );
			}
			
			PlayPosition = 0.0f;
			LastPlayPosition = 0.0f;
		}
		else if ( Playing )
		{
			PlayPosition = AudioPlayerInstance.Time();

			const float PlayDelta = PlayPosition - LastPlayPosition;
			ActualPosition = ActualPosition + PlayDelta * AudioPlayerSpatial.Rate;
			ActualPosition = std::fmod( ActualPosition, AudioPlayerInstance.Length() );

			LastPlayPosition = PlayPosition;
		}
	}

	ImGui::End();
}

static CRenderTexture ShaderToyTexture;
static CShader* ShaderToyShader = nullptr;
static CTexture* Slot0 = nullptr;
static bool Slot0External = false;
static CTexture* Slot1 = nullptr;
static bool Slot1External = false;
static CTexture* Slot2 = nullptr;
static bool Slot2External = false;

class CRenderPassShaderToy : public CRenderPass
{
public:
	CRenderPassShaderToy( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true ) : CRenderPass( "ShaderToy", Width, Height, Camera, AlwaysClear )
	{
		
	}

	virtual uint32_t Render( UniformMap& Uniforms ) override
	{
		auto& Assets = CAssets::Get();
		auto* Shader = Assets.Shaders.Find( "shadertoy" );
		if( !Shader )
			return 0;

		CRenderable ShaderToyRenderable;
		ShaderToyRenderable.SetMesh( Assets.Meshes.Find( "square" ) );
		ShaderToyRenderable.SetShader( Shader );

		if( Slot0 )
		{
			ShaderToyRenderable.SetTexture( Slot0, ETextureSlot::Slot0 );
		}
		else
		{
			ShaderToyRenderable.SetTexture( Assets.FindTexture( "rt_buffera" ), ETextureSlot::Slot0 );
		}

		if( Slot1 )
		{
			ShaderToyRenderable.SetTexture( Slot1, ETextureSlot::Slot1 );
		}

		if( Slot2 )
		{
			ShaderToyRenderable.SetTexture( Slot2, ETextureSlot::Slot2 );
		}

		return RenderRenderable( &ShaderToyRenderable, Uniforms );
	}
};

void ShaderToyUI()
{
	if( !ShowShaderToy )
		return;

	const ImGuiWindowFlags Flags = 0;
	if( ImGui::Begin( "Shader Toy", &ShowShaderToy, ImVec2( 950.0f, 1000.0f ), -1, Flags ) )
	{
		if( ImGui::Button( "..." ) )
		{
			DialogFormats Formats;
			Formats.insert_or_assign( L"Shader", L"*.fs;" );
			const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
			const CFile File( Path );
			if( File.Exists() )
			{
				auto* Shader = CAssets::Get().CreateNamedShader( "shadertoy", "Shaders/FullScreenQuad", File.Location( false ).c_str() );
				if( Shader )
				{
					Shader->Load( "Shaders/FullScreenQuad", File.Location( false ).c_str() );
					Shader->AutoReload( true );
					ShaderToyShader = Shader;
				}
			}
		}

		if( ShaderToyShader )
		{
			ImGui::SameLine();
			if( ImGui::Button( "Recompile" ) )
			{
				ShaderToyShader->Reload();
			}

			ImGui::SameLine();
			if( ImGui::Button( "Slot 0" ) )
			{
				DialogFormats Formats;
				Formats.insert_or_assign( L"Texture", L"*.png;*.jpg;*.tga;" );
				const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
				const CFile File( Path );
				if( File.Exists() )
				{
					if( Slot0External )
					{
						delete Slot0;
					}

					Slot0 = new CTexture( Path.c_str() );
					Slot0->Load();
					CAssets::Get().CreateNamedTexture( "shadertoy_slot0", Slot0 );
					Slot0External = true;
				}
			}

			if( ImGui::IsItemClicked( 1 ) && PreviewTexture )
			{
				if( Slot0External )
				{
					delete Slot0;
				}

				Slot0 = PreviewTexture;
				Slot0External = false;
			}

			ImGui::SameLine();
			if( ImGui::Button( "Slot 1" ) )
			{
				DialogFormats Formats;
				Formats.insert_or_assign( L"Texture", L"*.png;*.jpg;*.tga;" );
				const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
				const CFile File( Path );
				if( File.Exists() )
				{
					if( Slot1External )
					{
						delete Slot1;
					}

					Slot1 = new CTexture( Path.c_str() );
					Slot1->Load();
					CAssets::Get().CreateNamedTexture( "shadertoy_slot1", Slot1 );
					Slot1External = true;
				}
			}

			if( ImGui::IsItemClicked( 1 ) && PreviewTexture )
			{
				if( Slot1External )
				{
					delete Slot1;
				}

				Slot1 = PreviewTexture;
				Slot1External = false;
			}

			ImGui::SameLine();
			if( ImGui::Button( "Slot 2" ) )
			{
				DialogFormats Formats;
				Formats.insert_or_assign( L"Texture", L"*.png;*.jpg;*.tga;" );
				const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
				const CFile File( Path );
				if( File.Exists() )
				{
					if( Slot2External )
					{
						delete Slot2;
					}

					Slot2 = new CTexture( Path.c_str() );
					Slot2->Load();
					CAssets::Get().CreateNamedTexture( "shadertoy_slot2", Slot2 );
					Slot2External = true;
				}
			}

			if( ImGui::IsItemClicked( 1 ) && PreviewTexture )
			{
				if( Slot2External )
				{
					delete Slot2;
				}

				Slot2 = PreviewTexture;
				Slot2External = false;
			}
		}

		auto ImageSize = ImGui::GetWindowSize();
		ImageSize.x -= 20.0f;
		ImageSize.y -= 60.0f;

		auto* Handle = reinterpret_cast<ImTextureID>( ShaderToyTexture.GetHandle() );
		ImGui::Image( Handle, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
	}

	ImGui::End();

	if( !ShaderToyTexture.Ready() )
	{
		RenderTextureConfiguration Configuration;
		Configuration.Width = 1000;
		Configuration.Height = 1000;
		Configuration.Format = EImageFormat::RGB8;
		
		ShaderToyTexture = CRenderTexture( "ShaderToy", Configuration );
	}

	if( !ShaderToyShader )
		return;

	static CRenderPassShaderToy ShaderToyPass( 1000, 1000, CCamera() );
	ShaderToyPass.Target = &ShaderToyTexture;
	CWindow::Get().GetRenderer().AddRenderPass( &ShaderToyPass, RenderPassLocation::PreScene );
}

void TagListUI()
{
	if( !ShowTagList )
		return;

	if( ImGui::Begin( "Tag List", &ShowTagList, ImVec2( 500.0f, 700.0f ) ) )
	{
		// Fetch all of the tags from the primary world.
		std::unordered_map<std::string, std::vector<CEntity*>> Tags;
		if( const auto* World = CWorld::GetPrimaryWorld() )
		{
			Tags = World->GetTags();
		}

		ImGui::Columns( 2 );

		ImGui::Separator();
		ImGui::Text( "Tag" ); ImGui::NextColumn();
		ImGui::Text( "Entity" ); ImGui::NextColumn();

		ImGui::Separator();
		ImGui::Separator();

		for( const auto& Pair : Tags )
		{
			std::string NodeName = Pair.first + " (" + std::to_string( Pair.second.size() ) + ")";
			if( ImGui::TreeNode( NodeName.c_str() ) )
			{
				ImGui::NextColumn();
				ImGui::Text( "Debug" );

				ImGui::SameLine();

				int ForceDebug = 0;
				if( ImGui::Button( "Enable" ) )
				{
					ForceDebug = 1;
				}

				ImGui::SameLine();

				if( ImGui::Button( "Disable" ) )
				{
					ForceDebug = 2;
				}

				if( ForceDebug > 0 )
				{
					for( auto* Entity : Pair.second )
					{
						if( !Entity )
							continue;

						Entity->EnableDebug( ForceDebug == 1 );
					}
				}
				ImGui::NextColumn();

				for( auto* Entity : Pair.second )
				{
					if( Entity )
					{
						ImGui::Text( "%s", Entity->Identifier.ID );
						ImGui::NextColumn();

						if( ImGui::Selectable( Entity->Name.String().c_str(), Entity->IsDebugEnabled() ) )
						{
							Entity->EnableDebug( !Entity->IsDebugEnabled() );
						}
						ImGui::NextColumn();
					}
					else
					{
						ImGui::NextColumn();
						ImGui::Text( "nullptr" ); ImGui::NextColumn();
					}

					ImGui::Separator();
				}

				ImGui::TreePop();
			}

			ImGui::NextColumn();
			ImGui::NextColumn();
		}

		ImGui::Separator();
		ImGui::Separator();

		ImGui::Columns( 1 );
	}

	ImGui::End();
}

constexpr ImGuiColorEditFlags ColorPickerFlags =
ImGuiColorEditFlags_PickerHueWheel |
ImGuiColorEditFlags_NoSidePreview |
ImGuiColorEditFlags_NoLabel |
ImGuiColorEditFlags_Float
;

std::string VectorToString( const Vector4D& Vector )
{
	return std::to_string( Vector.X ) + "f, " + std::to_string( Vector.Y ) + "f, " + std::to_string( Vector.Z ) + "f, " + std::to_string( Vector.W ) + "f";
}

void PopulateGradePanel( ColorGrade& Grade )
{
	if( ImGui::Button( "Reset to Neutral" ) )
	{
		Grade.Tint = { 1.0f, 1.0f, 1.0f, DefaultExposure };
		Grade.Lift = { 0.0f, 0.0f, 0.0f, 0.0f };
		Grade.Gamma = { 0.0f, 0.0f, 0.0f, 0.0f };
		Grade.Gain = { 1.0f, 1.0f, 1.0f, 1.0f };
		Grade.BSC = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	ImGui::SameLine();

	if( ImGui::Button( "Copy to Clipboard" ) )
	{
		std::string Clipboard;
		Clipboard += ".Tint = {" + VectorToString( Grade.Tint ) + "}\n";
		Clipboard += ".Lift = {" + VectorToString( Grade.Lift ) + "}\n";
		Clipboard += ".Gamma = {" + VectorToString( Grade.Gamma ) + "}\n";
		Clipboard += ".Gain = {" + VectorToString( Grade.Gain ) + "}\n";
		Clipboard += ".BSC = {" + VectorToString( Grade.BSC ) + "}\n";
		glfwSetClipboardString( CWindow::Get().Handle(), Clipboard.c_str() );
	}

	ImGui::ColorPicker3( "Tint", &Grade.Tint.R, ColorPickerFlags );
	ImGui::DragFloat( "Exposure", &Grade.Tint.A, 0.01f, -10.0f, 10.0f, "%.2f" );

	ImGui::ColorPicker3( "Lift#ColorLift", &Grade.Lift.R, ColorPickerFlags );
	ImGui::DragFloat( "Lift", &Grade.Lift.A, 0.01f, -1.0f, 1.0f, "%.2f" );

	ImGui::ColorPicker3( "Gamma#ColorGamma", &Grade.Gamma.R, ColorPickerFlags );
	ImGui::DragFloat( "Gamma", &Grade.Gamma.A, 0.01f, -1.0f, 1.0f, "%.2f" );

	ImGui::ColorPicker3( "Gain#ColorGain", &Grade.Gain.R, ColorPickerFlags );
	ImGui::DragFloat( "Gain", &Grade.Gain.A, 0.01f, 0.0f, 16.0f, "%.2f" );

	ImGui::DragFloat( "Brightness", &Grade.BSC.R, 0.01f, 0.0f, 16.0f, "%.2f" );
	ImGui::DragFloat( "Saturation", &Grade.BSC.G, 0.01f, 0.0f, 16.0f, "%.2f" );
	ImGui::DragFloat( "Contrast", &Grade.BSC.B, 0.01f, 0.0f, 16.0f, "%.2f" );
}

static ColorGrade OverrideGrade;
void ColorManagementUI()
{
	if( !ShowColorManagement )
		return;

	auto& Renderer = GetRenderer();
	Renderer.Grade = OverrideGrade; // Assign the override grade directly.

	if( ImGui::Begin( "Color Management", &ShowColorManagement, ImVec2( 250.0f, 1000.0f ) ) )
	{
		PopulateGradePanel( OverrideGrade );
	}

	ImGui::End();
}

void PopulateCameraPanel()
{
	auto* World = CWorld::GetPrimaryWorld();
	if( !World )
	{
		ImGui::Text( "Primary world unavailable." );
		return;
	}

	auto* Camera = World->GetActiveCamera();
	if( !Camera )
	{
		ImGui::Text( "No active camera in primary world." );
		return;
	}

	ImGui::Text( "Camera 0x%p", Camera );
	ImGui::Checkbox( "Freeze", &Camera->Freeze );

	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "Freezes culling frustum update." );
		ImGui::EndTooltip();
	}

	const auto& CameraSetup = Camera->GetCameraSetup();
	ImGui::Columns( 3 );

	ImGui::Text( "Camera Location:" );
	ImGui::NextColumn();
	ImGui::Text( "%.2f %.2f %.2f", CameraSetup.CameraPosition[0], CameraSetup.CameraPosition[1], CameraSetup.CameraPosition[2] );
	ImGui::NextColumn();
	if( ImGui::Button( "Copy Location" ) )
	{
		std::stringstream Coordinate;
		Coordinate << CameraSetup.CameraPosition[0] << " " << CameraSetup.CameraPosition[1] << " " << CameraSetup.CameraPosition[2];
		glfwSetClipboardString( CWindow::Get().Handle(), Coordinate.str().c_str() );
	}
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::Text( "Camera Direction:" );
	ImGui::NextColumn();
	ImGui::Text( "%.2f %.2f %.2f", CameraSetup.CameraDirection[0], CameraSetup.CameraDirection[1], CameraSetup.CameraDirection[2] );
	ImGui::NextColumn();
	if( ImGui::Button( "Copy Direction" ) )
	{
		std::stringstream Coordinate;
		Coordinate << CameraSetup.CameraDirection[0] << " " << CameraSetup.CameraDirection[1] << " " << CameraSetup.CameraDirection[2];
		glfwSetClipboardString( CWindow::Get().Handle(), Coordinate.str().c_str() );
	}
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::Text( "Camera Orientation:" );
	ImGui::NextColumn();
	ImGui::Text( "%.2f %.2f %.2f", Camera->CameraOrientation[0], Camera->CameraOrientation[2], Camera->CameraOrientation[1] );
	ImGui::NextColumn();
	if( ImGui::Button( "Copy Orientation" ) )
	{
		std::stringstream Coordinate;
		Coordinate << Camera->CameraOrientation[0] << " " << Camera->CameraOrientation[2] << " " << Camera->CameraOrientation[1];
		glfwSetClipboardString( CWindow::Get().Handle(), Coordinate.str().c_str() );
	}
	ImGui::NextColumn();

	ImGui::Separator();

	ImGui::Text( "Camera FOV:" );
	ImGui::NextColumn();
	ImGui::Text( "%.2f", CameraSetup.FieldOfView );
	ImGui::NextColumn();
	if( ImGui::Button( "Copy FOV" ) )
	{
		std::stringstream FOV;
		FOV << CameraSetup.FieldOfView;
		glfwSetClipboardString( CWindow::Get().Handle(), FOV.str().c_str() );
	}
	ImGui::NextColumn();
}

void CameraInformationUI()
{
	if( !ShowCameraInformation )
		return;

	if( ImGui::Begin( "Camera Information", &ShowCameraInformation, ImVec2( 400.0f, 400.0f ) ) )
	{
		PopulateCameraPanel();
	}

	ImGui::End();
}

bool PerpetualRecompile = false;
double LastRecompile = -1.0;
void PerformPerpetualRecompile()
{
	if( !PerpetualRecompile )
		return;

	const auto Time = GameLayersInstance->GetRealTime();
	if( LastRecompile < 0.0 )
		LastRecompile = Time;

	const auto DeltaTime = Time - LastRecompile;
	if( DeltaTime > 2.0 )
	{
		LastRecompile = Time;
		CAssets::Get().ReloadShaders();
	}
}

void ForceAnisotropicSamples( const uint8_t Samples )
{
	for( auto* Texture : CAssets::Get().Textures.GetAssets() )
	{
		if( !Texture )
			continue;

		Texture->SetAnisotropicSamples( Samples );
	}
}

extern ConfigurationVariable<bool> FlipHorizontal;
void RenderCommandItems()
{
	if( ImGui::MenuItem( "Flip Horizontal", nullptr, FlipHorizontal.Get() ) )
	{
		FlipHorizontal.Set( !FlipHorizontal.Get() );
	}

	if( ImGui::MenuItem( "Perpetual Shader Compilation", nullptr, PerpetualRecompile ) )
	{
		PerpetualRecompile = !PerpetualRecompile;
	}

	if( ImGui::IsItemHovered() )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "Recompile shaders every 2 seconds." );
		ImGui::EndTooltip();
	}

	if( ImGui::MenuItem( "Force Anisotropic Filtering Off" ) )
	{
		ForceAnisotropicSamples( 1 );
	}

	if( ImGui::MenuItem( "Force Anisotropic Filtering x2" ) )
	{
		ForceAnisotropicSamples( 2 );
	}

	if( ImGui::MenuItem( "Force Anisotropic Filtering x4" ) )
	{
		ForceAnisotropicSamples( 4 );
	}

	if( ImGui::MenuItem( "Force Anisotropic Filtering x8" ) )
	{
		ForceAnisotropicSamples( 8 );
	}

	if( ImGui::MenuItem( "Force Anisotropic Filtering x16" ) )
	{
		ForceAnisotropicSamples( 16 );
	}
}

void RenderWindowItems()
{
	if( MenuItem( "Assets", DisplayAssets() ) )
	{
		SetPreviewTexture( nullptr );
	}

	MenuItem( "Strings", DisplayStrings() );
	MenuItem( "Mixer", DisplayMixer() );
	MenuItem( "Audio Player", DisplayAudioPlayer() );
	MenuItem( "Shader Toy", DisplayShaderToy() );
	MenuItem( "Tag List", DisplayTagList() );
	MenuItem( "Color Management", DisplayColorManagement() );
	MenuItem( "Camera Information", DisplayCameraInformation() );
}

void RenderWindowPanels()
{
	AssetUI();
	StringUI();
	MixerUI();
	AudioPlayerUI();
	ShaderToyUI();
	TagListUI();
	ColorManagementUI();
	CameraInformationUI();

	PerformPerpetualRecompile();
}

#if defined(_WIN32)
#include <Windows.h>
#include <shlobj.h>
#include <objbase.h> // COM headers
#include <shobjidl.h>  // IFileOpenDialog

#undef GetCurrentTime
#endif

std::string OpenFileDialog( const DialogFormats& Formats )
{
	std::string InvalidFile;
#if defined(_WIN32)
	HRESULT hr = CoInitializeEx( nullptr, 0 );
	if( !SUCCEEDED( hr ) )
		return InvalidFile;

	IFileOpenDialog* FileDialog;

	// Create the FileOpenDialog object.
	hr = CoCreateInstance( CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
		IID_IFileOpenDialog, reinterpret_cast<void**>( &FileDialog ) );

	if( !SUCCEEDED( hr ) )
	{
		FileDialog->Release();
		CoUninitialize();
		return InvalidFile;
	}

	auto* FilterSpecification = new COMDLG_FILTERSPEC[Formats.size()];
	int FormatIndex = 0;
	for( auto& Format : Formats )
	{
		FilterSpecification[FormatIndex].pszName = Format.first.c_str();
		FilterSpecification[FormatIndex].pszSpec = Format.second.c_str();
		FormatIndex++;
	}

	/*DWORD dwFlags;
	pFileOpen->GetOptions( &dwFlags );
	pFileOpen->SetOptions( dwFlags | FOS_ALLOWMULTISELECT );*/
	
	hr = FileDialog->SetFileTypes( 1, FilterSpecification );

	delete FilterSpecification;

	if( !SUCCEEDED( hr ) )
		return InvalidFile;

	// Show the Open dialog box.
	hr = FileDialog->Show( nullptr );

	// Get the file name from the dialog box.
	if( !SUCCEEDED( hr ) )
		return InvalidFile;
	
	IShellItem* pItem;
	hr = FileDialog->GetResult( &pItem );
	if( SUCCEEDED( hr ) )
	{
		PWSTR pszFilePath;
		hr = pItem->GetDisplayName( SIGDN_FILESYSPATH, &pszFilePath );

		// Display the file name to the user.
		if( SUCCEEDED( hr ) )
		{
			size_t Converted;
			char FilePath[256];
			auto Result = wcstombs_s( &Converted, FilePath, pszFilePath, sizeof( FilePath ) );
			if( Result == 256 )
			{
				FilePath[255] = '\0';
			}

			CoTaskMemFree( pszFilePath );

			return FilePath;
		}
		pItem->Release();
	}

	FileDialog->Release();
	CoUninitialize();
#endif

	return InvalidFile;
}

Bus::Type ImGui::BusSelector( const Bus::Type& Bus )
{
	const auto& CurrentBus = TranslateBus.From( Bus );
	Bus::Type NewBus = Bus;
	if( BeginCombo( "Bus", CurrentBus.c_str() ) )
	{
		const auto& Keys = TranslateBus.Keys();
		const auto& Values = TranslateBus.Values();

		for( size_t Index = 0; Index < Keys.size(); Index++ )
		{
			if( Selectable( Keys[Index].c_str() ) )
			{
				NewBus = Values[Index];
			}
		}

		EndCombo();
	}

	return NewBus;
}
