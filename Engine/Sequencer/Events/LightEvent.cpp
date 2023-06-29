// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "LightEvent.h"

#include <Engine/Application/ApplicationMenu.h>
#include <Engine/Utility/Serialize.h>
#include <Engine/World/World.h>

#include <ThirdParty/imgui-1.70/imgui.h>

void CopyCameraDataToLight( Light& Information )
{
	const auto* World = CWorld::GetPrimaryWorld();
	if( !World )
		return;

	const auto* ActiveCamera = World->GetActiveCamera();
	if( !ActiveCamera )
		return;

	auto& Setup = ActiveCamera->GetCameraSetup();
	Information.Position.x = Setup.CameraPosition.X;
	Information.Position.y = Setup.CameraPosition.Y;
	Information.Position.z = Setup.CameraPosition.Z;

	Information.Direction.x = Setup.CameraDirection.X;
	Information.Direction.y = Setup.CameraDirection.Y;
	Information.Direction.z = Setup.CameraDirection.Z;
}

void LightEvent::Evaluate( const Timecode& Marker )
{
	if( InRange( Marker ) )
	{
		Execute();
	}
	else
	{
		// HACK: "Disables" the light.
		LightEntity::Get( LightIndex ).Color.w = 0.0f;
	}
}

void LightEvent::Execute()
{
	if( LightIndex == -1 )
	{
		LightIndex = LightEntity::AllocateLight();

		if( LightIndex == -1 )
			return; // Out of lights.
	}

	LightEntity::Get( LightIndex ) = Information;
}

constexpr ImGuiColorEditFlags ColorPickerFlags =
ImGuiColorEditFlags_PickerHueWheel |
ImGuiColorEditFlags_NoSidePreview |
ImGuiColorEditFlags_NoLabel |
ImGuiColorEditFlags_Float
;

const char* LightType[] = {
	"Point",
	"Spot",
	"Directional"
};

void LightEvent::Context()
{
	ImGui::Text( "Light" );
	ImGui::ColorPicker3( "Color", &Information.Color.r, ColorPickerFlags );
	ImGui::DragFloat( "Intensity", &Information.Color.w, 0.1f, 0.0f, 10000.0f, "%.2f" );
	ImGui::DragFloat( "Radius", &Information.Direction.w, 0.1f, 0.01f, 1000.0f, "%.2f" );

	if( ImGui::Button( "Copy Camera Position" ) )
	{
		CopyCameraDataToLight( Information );
	}

	ImGui::DragFloat3( "Position", &Information.Position.x, 0.1f );

	int Type = static_cast<int>( Information.Position.w );
	if( Type > 0 )
	{
		ImGui::DragFloat3( "Direction", &Information.Direction.x, 0.1f );
		Information.Direction = glm::normalize( Information.Direction );
	}

	if( ImGui::BeginCombo( "LightType", LightType[Type] ) )
	{
		for( int Index = 0; Index < 3; Index++ )
		{
			if( ImGui::Selectable( LightType[Index] ) )
			{
				Information.Position.w = static_cast<int>( Index );
			}
		}

		ImGui::EndCombo();
	}

	if( Type != 1 )
		return; // Not a spotlight.

	ImGui::DragFloat( "Inner Cone", &Information.Properties.x, 0.01f, 0.0f, 180.0f, "%.2f" );
	ImGui::DragFloat( "Outer Cone", &Information.Properties.y, 0.01f, 0.0f, 180.0f, "%.2f" );
}

constexpr const char* GradeName = "Light";
const char* LightEvent::GetName()
{
	return GradeName;
}

const char* LightEvent::GetType() const
{
	return GradeName;
}

void LightEvent::Export( CData& Data ) const
{
	Serialize::Export( Data, "pos", Information.Position );
	Serialize::Export( Data, "dir", Information.Direction );
	Serialize::Export( Data, "col", Information.Color );
	Serialize::Export( Data, "prp", Information.Properties );
}

void LightEvent::Import( CData& Data )
{
	Serialize::Import( Data, "pos", Information.Position );
	Serialize::Import( Data, "dir", Information.Direction );
	Serialize::Import( Data, "col", Information.Color );
	Serialize::Import( Data, "prp", Information.Properties );
}
