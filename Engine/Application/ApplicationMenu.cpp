// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "ApplicationMenu.h"

#include <Engine/Application/Application.h>
#include <Engine/Audio/Sound.h>
#include <Engine/Display/Window.h>
#include <Engine/Display/Rendering/Camera.h>
#include <Engine/Display/Rendering/Renderable.h>
#include <Engine/Display/Rendering/Texture.h>
#include <Engine/Display/Rendering/RenderPass.h>
#include <Engine/Resource/Assets.h>
#include <Engine/Resource/Asset.h>
#include <Engine/Sequencer/Sequencer.h>
#include <Engine/World/World.h>

#include <Game/Game.h>

#if defined( IMGUI_ENABLED )
#include <ThirdParty/imgui-1.70/imgui.h>
#include <Engine/Display/imgui_impl_glfw.h>
#include <Engine/Display/imgui_impl_opengl3.h>
#endif

CMesh* PreviewMesh = nullptr;
CRenderTexture* PreviewRenderTexture;
class CModelPass : public CRenderPass
{
public:
	CModelPass()
		: CRenderPass( "ModelPass", 512, 512, CCamera(), false )
	{
		
	}

	uint32_t Render( const UniformMap& Uniforms ) override
	{
		if( !Mesh )
			return 0;
		
		auto& Assets = CAssets::Get();

		const auto Bounds = Mesh->GetBounds();
		const auto Center = ( Bounds.Maximum + Bounds.Minimum ) * 0.5f;
		const auto Extent = ( Bounds.Maximum - Bounds.Minimum ) * 0.5f;
		const auto MaximumDistance = Math::VectorMax( Extent.X, Extent.Y, Extent.Z );

		const auto Time = StaticCast<float>( GameLayersInstance->GetRealTime() );
		
		auto& Setup = Camera.GetCameraSetup();
		Setup.CameraPosition = Center + Vector3D( MaximumDistance * 2.0f * sin( Time ), MaximumDistance * 2.0f * cos( Time ), MaximumDistance );
		Setup.CameraDirection = Center - Setup.CameraPosition;
		Setup.NearPlaneDistance = 0.01f;
		Setup.FarPlaneDistance = Setup.CameraDirection.Length() * 2.0f;
		Setup.CameraDirection.Normalize();
		Setup.FieldOfView = 55.0f;
		Setup.AspectRatio = 1.1f;
		Camera.Update();

		CRenderable Model;
		Model.SetMesh( Mesh );
		Model.SetShader( Assets.FindShader( "default" ) );
		Model.SetTexture( Assets.FindTexture( "error" ), ETextureSlot::Slot0 );
		auto& RenderData = Model.GetRenderData();
		RenderData.Transform = FTransform();

		ClearTarget();

		return RenderRenderable( &Model, Uniforms );
	}

	CMesh* Mesh = nullptr;
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

CTexture* GenerateThumbnail( const std::string& Name, CMesh* Mesh )
{
	RenderTextureConfiguration Configuration;
	Configuration.Width = 64;
	Configuration.Height = 64;
	Configuration.Format = EImageFormat::RGBA8;

	auto* ThumbnailTexture = new CRenderTexture( "ModelThumbnail", Configuration );
	Thumbnails.insert_or_assign( Name, ThumbnailTexture );

	static CModelPass ModelPass;
	ModelPass.Target = ThumbnailTexture;
	ModelPass.Mesh = Mesh;

	auto& Window = CWindow::Get();
	CRenderer& Renderer = Window.GetRenderer();
	Renderer.AddRenderPass( &ModelPass, ERenderPassLocation::PreScene );
	
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
		ModelPass.Target = PreviewRenderTexture;
		ModelPass.Mesh = PreviewMesh;
		PreviewTexture = PreviewRenderTexture;
		Renderer.AddRenderPass( &ModelPass, ERenderPassLocation::PreScene );
	}

	if( !Texture )
		return;
	
	auto ImageSize = ImVec2( Texture->GetWidth(), Texture->GetHeight() );
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
	ImGui::Image( TextureID, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
	ImGui::SetCursorPos( ContentPosition );
	ImGui::InvisibleButton( "PreviewPanel", ImageSize );

	if( ImGui::IsWindowHovered() )
	{
		float TextureX = ( ( ImGui::GetMousePos().x - CursorPosition.x ) / ImageSize.x ) * Texture->GetWidth();
		float TextureY = ( ( ImGui::GetMousePos().y - CursorPosition.y ) / ImageSize.y ) * Texture->GetHeight();

		ImGui::BeginTooltip();
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
}

CTexture* UIFileSpeaker = nullptr; // Textures/UI/round_volume_up_black_48.png

CTexture* CreateUIIcon( const char* Name, const char* Location )
{
	return CAssets::Get().CreateNamedTexture( Name, Location, EFilteringMode::Linear, EImageFormat::RGBA8 );
}

bool IconsCreated = false;
void CreateIcons()
{
	if( IconsCreated )
		return;
	
	UIFileSpeaker = CreateUIIcon( "ui_speaker", "Textures/UI/round_volume_up_black_48.png" );

	IconsCreated = true;
}

static bool ShowTextures = true;
static bool ShowSounds = true;
static bool ShowMeshes = true;
static char FilterText[512] = {};

bool MatchFilter( const char* Name )
{
	return std::strstr( Name, FilterText ) != nullptr;
}

void ContentBrowserUI()
{
	CreateIcons();
	ImGui::BeginChild( "Content Panel", ImVec2( 0, ImGui::GetContentRegionAvail().y * 0.5f ) );
	const int Columns = std::max( static_cast<int>( ImGui::GetWindowWidth() / 66 ), 1 );
	ImGui::Columns( Columns );

	const auto FilterLength = std::strlen( FilterText );

	auto& Assets = CAssets::Get();
	if( ShowTextures )
	{
		const auto& Textures = Assets.GetTextures();
		for( const auto& Pair : Textures )
		{
			if( FilterLength > 0 && !MatchFilter( Pair.first.c_str() ) )
				continue;
			
			CTexture* Texture = Pair.second;
			if( Texture )
			{
				auto ImageSize = ImVec2( 64, 64 );

				auto* TextureID = reinterpret_cast<ImTextureID>( Texture->GetHandle() );
				if( ImGui::ImageButton( TextureID, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1 ) )
				{
					PreviewName = Pair.first;
					PreviewTexture = Texture;
					PreviewMesh = nullptr;
				}

				if( ImGui::IsItemHovered() )
				{
					ImGui::BeginTooltip();
					ImGui::Text( "%s", Pair.first.c_str() );
					ImGui::Text( "%s", Texture->GetLocation().c_str() );
					ImGui::Text( "%ix%i", Texture->GetWidth(), Texture->GetHeight() );
					ImGui::Text( "%s", Assets.GetReadableImageFormat( Texture->GetImageFormat() ).c_str() );
					ImGui::EndTooltip();
				}

				ImGui::NextColumn();
			}
		}
	}

	if( ShowSounds )
	{
		const auto& Sounds = Assets.GetSounds();
		for( const auto& Pair : Sounds )
		{
			if( FilterLength > 0 && !MatchFilter( Pair.first.c_str() ) )
				continue;
			
			CSound* Sound = Pair.second;
			if( Sound )
			{
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
					PreviewTexture = nullptr;
					PreviewMesh = nullptr;

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
					ImGui::BeginTooltip();
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
	}

	if( ShowMeshes )
	{
		const auto& Meshes = Assets.GetMeshes();
		for( const auto& Pair : Meshes )
		{
			if( FilterLength > 0 && !MatchFilter( Pair.first.c_str() ) )
				continue;

			CMesh* Mesh = Pair.second;
			if( !Mesh )
				continue;

			auto ImageSize = ImVec2( 64, 64 );

			bool ShouldGenerateThumbnail = false;
			CTexture* Thumbnail = GetThumbnail( Pair.first );
			if( !Thumbnail )
			{
				ShouldGenerateThumbnail = true;
			}

			if( !Thumbnail )
			{
				Thumbnail = UIFileSpeaker;
			}
			
			auto* TextureID = reinterpret_cast<ImTextureID>( Thumbnail->GetHandle() );
			const ImVec4 Background = ImVec4( 0, 0, 0, 0 );
			const ImVec4 Tint = ShouldGenerateThumbnail ? ImVec4( 0, 1.0f, 0, 1.0f ) : ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

			ImGui::PushID( Pair.first.c_str() );
			if( ImGui::ImageButton(
				TextureID, ImageSize, ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1,
				Background,
				Tint ) )
			{
				PreviewName = Pair.first;
				PreviewTexture = nullptr;
				PreviewMesh = Mesh;
			}

			if( ImGui::IsItemHovered() )
			{
				ImGui::BeginTooltip();
				ImGui::Text( "%s", Pair.first.c_str() );
				ImGui::EndTooltip();

				if( ShouldGenerateThumbnail )
				{
					GenerateThumbnail( Pair.first, Pair.second );
				}
			}

			ImGui::PopID();

			ImGui::NextColumn();
		}
	}

	ImGui::Columns( 1 );
	ImGui::EndChild();

	ImGui::BeginChild( "Texture Preview", ImVec2( 0, 0 ), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
	ShowTexture( PreviewTexture );
	ImGui::EndChild();
}

static bool ShowNewAssetPopup = false;
static std::string NewAssetType = "Sequence";
static std::string NewAssetName = "NewAsset";

void NewAssetUI()
{
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

void AssetUI()
{
	auto& Assets = CAssets::Get();

	if( ShowAssets )
	{
		if( ImGui::Begin( "Assets", &ShowAssets, ImVec2( 1000.0f, 700.0f ) ) )
		{
			if( ImGui::Button( "New##NewAssetPopup" ) )
			{
				ShowNewAssetPopup = true;
			}

			NewAssetUI();

			ImGui::SameLine(); ImGui::Checkbox( "Textures", &ShowTextures );
			ImGui::SameLine(); ImGui::Checkbox( "Sounds", &ShowSounds );
			ImGui::SameLine(); ImGui::Checkbox( "Meshes", &ShowMeshes );
			ImGui::InputText( "Filter", FilterText, 512 );

			ImGui::BeginChild( "Asset List", ImVec2( ImGui::GetWindowContentRegionWidth() * 0.33f, ImGui::GetContentRegionAvail().y ) );
			ImGui::Columns( 2 );

			ImGui::Separator();
			ImGui::Text( "Name" ); ImGui::NextColumn();
			ImGui::Text( "Type" ); ImGui::NextColumn();

			ImGui::Separator();
			ImGui::Separator();

			auto World = CWorld::GetPrimaryWorld();
			if( World )
			{
				auto& Levels = World->GetLevels();
				for( auto& Level : Levels )
				{
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

			auto CustomAssets = Assets.GetAssets();
			for( const auto& Pair : CustomAssets )
			{
				if( ImGui::Selectable( Pair.first.c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
				{

				}
				ImGui::NextColumn();

				ImGui::Text( Pair.second->GetType().c_str() ); ImGui::NextColumn();
				ImGui::Separator();
			}

			ImGui::Separator();
			ImGui::Separator();

			auto Meshes = Assets.GetMeshes();
			for( const auto& Pair : Meshes )
			{
				if( ImGui::Selectable( Pair.first.c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
				{
					auto& Assets = CAssets::Get();
					if( Pair.second && Assets.FindMesh( Pair.first ) )
					{
						CTimer LoadTimer;

						LoadTimer.Start();
						auto Mesh = Assets.CreateNamedMesh( Pair.first.c_str(), Pair.second->GetLocation().c_str(), true );
						LoadTimer.Stop();

						const size_t Triangles = Mesh->GetVertexBufferData().IndexCount / 3;

						Log::Event( "Re-import: %ims %i triangles\n", LoadTimer.GetElapsedTimeMilliseconds(), Triangles );
					}
				}
				ImGui::NextColumn();
				ImGui::Text( "Mesh" ); ImGui::NextColumn();
				ImGui::Separator();
			}

			ImGui::Separator();

			auto Shaders = Assets.GetShaders();
			for( const auto& Pair : Shaders )
			{
				// ImGui::Text( Pair.first.c_str() ); ImGui::NextColumn();
				if( ImGui::Selectable( Pair.first.c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
				{
					Pair.second->Reload();
				}
				ImGui::NextColumn();

				ImGui::Text( "Shader" ); ImGui::NextColumn();
				ImGui::Separator();
			}

			ImGui::Separator();

			auto Textures = Assets.GetTextures();
			for( const auto& Pair : Textures )
			{
				// ImGui::Text( Pair.first.c_str() ); ImGui::NextColumn();
				if( ImGui::Selectable( Pair.first.c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
				{
					PreviewTexture = Pair.second;
				}
				ImGui::NextColumn();

				ImGui::Text( "Texture" ); ImGui::NextColumn();
				ImGui::Separator();
			}

			ImGui::Separator();

			auto Sounds = Assets.GetSounds();
			for( const auto& Pair : Sounds )
			{
				if( ImGui::Selectable( Pair.first.c_str(), Pair.second->Playing(), ImGuiSelectableFlags_SpanAllColumns ) )
				{
					if( Pair.second->Playing() )
					{
						Pair.second->Stop();
					}
					else
					{
						Pair.second->Start();
					}
				}
				ImGui::NextColumn();

				// ImGui::Text( Pair.first.c_str() ); ImGui::NextColumn();
				ImGui::Text( "Sound" ); ImGui::NextColumn();
				ImGui::Separator();
			}

			auto Sequences = Assets.GetSequences();
			for( const auto& Pair : Sequences )
			{
				if( ImGui::Selectable( Pair.first.c_str(), false, ImGuiSelectableFlags_SpanAllColumns ) )
				{
					Pair.second->Draw();
				}
				ImGui::NextColumn();

				ImGui::Text( "Sequence" ); ImGui::NextColumn();
				ImGui::Separator();
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

	auto Sequences = Assets.GetSequences();
	for( const auto& Pair : Sequences )
	{
		Pair.second->Frame();
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

			for( const auto& Pair : FName::Pool() )
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

static bool ShowShaderToy = false;
bool* DisplayShaderToy()
{
	return &ShowShaderToy;
}

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
	
	const auto Volume = CSoLoudSound::GetBusOutput( Bus );
	if( BusEntry.find( Name ) == BusEntry.end() )
	{
		BusInformation Information;
		Information.Volume = Volume;
		Information.PeakLeft = Volume.Left;
		Information.PeakRight = Volume.Right;
		BusEntry.insert_or_assign( Name, Information );
	}
	
	const float MaximumVolume = CSoLoudSound::Volume( Bus );

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
	auto BaseColor = ImColor( 0.0f, 0.0f, 0.25f, 0.25f );
	auto LeftColor = ImColor( Math::Lerp( 0.0f, 1.0f, std::powf( ScaleLeft, 2.2f ) ), 0.0f, Math::Lerp( 2.0f, 0.0f, std::powf( ScaleLeft, 2.2f ) ), 0.75f );
	auto RightColor = ImColor( Math::Lerp( 0.0f, 1.0f, std::powf( ScaleRight, 2.2f ) ), 0.0f, Math::Lerp( 2.0f, 0.0f, std::powf( ScaleRight, 2.2f ) ), 0.75f );
	
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
			CSoLoudSound::Volume( Bus, Math::Saturate( MaximumVolume - Drag ) );
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
		CSoLoudSound::Volume( Bus, CSoLoudSound::Volume( Bus ) > 0.001f ? 0.0f : 1.0f );
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

static CRenderTexture ShaderToyTexture;
static CShader* ShaderToyShader = nullptr;
class CRenderPassShaderToy : public CRenderPass
{
public:
	CRenderPassShaderToy( int Width, int Height, const CCamera& Camera, const bool AlwaysClear = true ) : CRenderPass( "ShaderToy", Width, Height, Camera, AlwaysClear )
	{
		
	}

	virtual uint32_t Render( const UniformMap& Uniforms ) override
	{
		auto& Assets = CAssets::Get();
		auto* Shader = Assets.FindShader( "shadertoy" );
		if( !Shader )
			return 0;

		CRenderable ShaderToyRenderable;
		ShaderToyRenderable.SetMesh( Assets.FindMesh( "square" ) );
		ShaderToyRenderable.SetShader( Shader );

		return RenderRenderable( &ShaderToyRenderable, Uniforms );
	}
};

void ShaderToyUI()
{
	if( !ShowShaderToy )
		return;

	const ImGuiWindowFlags Flags = 0;
	if( ImGui::Begin( "Shader Toy", &ShowMixer, ImVec2( 950.0f, 1000.0f ), -1, Flags ) )
	{
		if( ImGui::Button( "..." ) )
		{
			DialogFormats Formats;
			Formats.insert_or_assign( L"Shader", L"*.fs;" );
			const std::string Path = CApplication::Relative( OpenFileDialog( Formats ) );
			const CFile File( Path.c_str() );
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
		}

		auto* Handle = reinterpret_cast<ImTextureID>( ShaderToyTexture.GetHandle() );
		ImGui::Image( Handle, ImVec2( 950.0f, 950.0f ), ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
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
	CWindow::Get().GetRenderer().AddRenderPass( &ShaderToyPass, ERenderPassLocation::PreScene );
}

void RenderMenuItems()
{
	if( MenuItem( "Assets", DisplayAssets() ) )
	{
		SetPreviewTexture( nullptr );
	}

	MenuItem( "Strings", DisplayStrings() );
	MenuItem( "Mixer", DisplayMixer() );
	MenuItem( "Shader Toy", DisplayShaderToy() );
}

void RenderMenuPanels()
{
	AssetUI();
	StringUI();
	MixerUI();
	ShaderToyUI();
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
#if defined(_WIN32)
	HRESULT hr = CoInitializeEx( NULL, 0 );
	if( SUCCEEDED( hr ) )
	{
		IFileOpenDialog* pFileOpen;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>( &pFileOpen ) );

		if( SUCCEEDED( hr ) )
		{
			COMDLG_FILTERSPEC* rgSpec = new COMDLG_FILTERSPEC[Formats.size()];
			int FormatIndex = 0;
			for( auto& Format : Formats )
			{
				rgSpec[FormatIndex].pszName = Format.first.c_str();
				rgSpec[FormatIndex].pszSpec = Format.second.c_str();
				FormatIndex++;
			}
			
			hr = pFileOpen->SetFileTypes( 1, rgSpec );

			delete rgSpec;

			if( SUCCEEDED( hr ) )
			{
				// Show the Open dialog box.
				hr = pFileOpen->Show( NULL );

				// Get the file name from the dialog box.
				if( SUCCEEDED( hr ) )
				{
					IShellItem* pItem;
					hr = pFileOpen->GetResult( &pItem );
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
				}
			}

			pFileOpen->Release();
		}
		CoUninitialize();
	}
#endif

	return "";
}
