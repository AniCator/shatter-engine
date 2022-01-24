// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include "ThemeDefault.h"
#include <ThirdParty/imgui-1.70/imgui.h>

#include <Engine/Display/Color.h>

/// Gruvbox Dark Theme
// Based on: https://github.com/morhetz/gruvbox

namespace Gruvbox
{
	namespace Dark
	{
		// Regular
		namespace Regular
		{
			Color Red = Color::FromHex( "#cc241d" );
			Color Green = Color::FromHex( "#98971a" );
			Color Yellow = Color::FromHex( "#d79921" );
			Color Orange = Color::FromHex( "#d65d0e" );
			Color Blue = Color::FromHex( "#458588" );
			Color Purple = Color::FromHex( "#b16286" );
			Color Aqua = Color::FromHex( "#689d6a" );
			Color Gray = Color::FromHex( "#a89984" );
		}

		namespace Active
		{
			Color Red = Color::FromHex( "#fb4934" );
			Color Green = Color::FromHex( "#b8bb26" );
			Color Yellow = Color::FromHex( "#fabd2f" );
			Color Orange = Color::FromHex( "#fe8019" );
			Color Blue = Color::FromHex( "#83a598" );
			Color Purple = Color::FromHex( "#d3869b" );
			Color Aqua = Color::FromHex( "#8ec07c" );
			Color Gray = Color::FromHex( "#928374" );
		}

		namespace Background
		{
			Color Default = Color::FromHex( "#282828" );
			Color Bright = Color::FromHex( "#7c6f64" );
			Color Neutral = Color::FromHex( "#665c54" );
			Color Faint = Color::FromHex( "#504945" );
			Color Dim = Color::FromHex( "#3c3836" );
		}

		namespace Foreground
		{
			Color Default = Color::FromHex( "#ebdbb2" );
			Color Bright = Color::FromHex( "#fbf1c7" );
			Color Neutral = Color::FromHex( "#d5c4a1" );
			Color Faint = Color::FromHex( "#bdae93" );
			Color Dim = Color::FromHex( "#a89984" );
		}
	}
}

using namespace Gruvbox;

ImColor Generate( const Color& Input )
{
	return { IM_COL32( Input.R, Input.G, Input.B, Input.A ) };
}

void Darken( ImVec4& Input, const float& Value )
{
	const auto Clamped = Value < 0.0f ? 0.0f : Value > 1.0f ? 1.0f : Value;
	Input.x *= Clamped;
	Input.y *= Clamped;
	Input.z *= Clamped;
}

void ThemeGruvboxDark()
{
	ThemeDefault();

	auto& Style = ImGui::GetStyle();
	Style.ChildRounding = 0;
	Style.GrabRounding = 0;
	Style.FrameRounding = 0;
	Style.PopupRounding = 0;
	Style.ScrollbarRounding = 0;
	Style.TabRounding = 0;
	Style.WindowRounding = 0;
	Style.FramePadding = { 4, 4 };

	Style.WindowTitleAlign = { 0.5, 0.5 };

	ImVec4* Colors = ImGui::GetStyle().Colors;

	Colors[ImGuiCol_Text] = Generate( Dark::Foreground::Neutral );
	Colors[ImGuiCol_TextDisabled] = Generate( Dark::Foreground::Dim );

	Colors[ImGuiCol_WindowBg] = Generate( Dark::Background::Dim );
	Darken( Colors[ImGuiCol_WindowBg], 0.9f );
	Colors[ImGuiCol_ChildBg] = Generate( Dark::Background::Dim );
	Colors[ImGuiCol_PopupBg] = Colors[ImGuiCol_WindowBg];

	Colors[ImGuiCol_Border] = Generate( Dark::Regular::Red );
	Colors[ImGuiCol_Border].w = 0.25f;
	Darken( Colors[ImGuiCol_Border], 0.25f );
	Colors[ImGuiCol_BorderShadow] = Generate( Dark::Regular::Red );
	Darken( Colors[ImGuiCol_BorderShadow], 0.65f );

	Colors[ImGuiCol_FrameBg] = Generate( Dark::Background::Faint );
	Colors[ImGuiCol_FrameBgHovered] = Generate( Dark::Background::Neutral );
	Colors[ImGuiCol_FrameBgActive] = Generate( Dark::Background::Bright );

	Colors[ImGuiCol_TitleBg] = Generate( Dark::Regular::Red );
	Darken( Colors[ImGuiCol_TitleBg], 0.6f );

	Colors[ImGuiCol_TitleBgActive] = Generate( Dark::Regular::Red );
	Colors[ImGuiCol_TitleBgCollapsed] = Generate( Dark::Regular::Red );
	Darken( Colors[ImGuiCol_TitleBgCollapsed], 0.4f );

	Colors[ImGuiCol_MenuBarBg] = Generate( Dark::Background::Dim );

	Colors[ImGuiCol_ScrollbarBg] = Generate( Dark::Background::Default );
	Colors[ImGuiCol_ScrollbarGrab] = Generate( Dark::Background::Faint );
	Colors[ImGuiCol_ScrollbarGrabHovered] = Generate( Dark::Background::Neutral );
	Colors[ImGuiCol_ScrollbarGrabActive] = Generate( Dark::Foreground::Dim );

	Colors[ImGuiCol_CheckMark] = Generate( Dark::Active::Yellow );

	Colors[ImGuiCol_SliderGrab] = Generate( Dark::Regular::Blue );
	Colors[ImGuiCol_SliderGrabActive] = Generate( Dark::Active::Blue );

	Colors[ImGuiCol_Button] = Generate( Dark::Active::Aqua );
	Colors[ImGuiCol_Button].w = 0.25f;
	Colors[ImGuiCol_ButtonHovered] = Generate( Dark::Active::Blue );
	Colors[ImGuiCol_ButtonActive] = Generate( Dark::Regular::Yellow );

	Colors[ImGuiCol_Header] = Generate( Dark::Regular::Yellow );
	Colors[ImGuiCol_Header].w = 0.25f;
	Colors[ImGuiCol_HeaderHovered] = Generate( Dark::Background::Bright );
	Colors[ImGuiCol_HeaderHovered].w = 0.35f;
	Colors[ImGuiCol_HeaderActive] = Generate( Dark::Background::Bright );

	Colors[ImGuiCol_Separator] = Generate( Dark::Background::Faint );
	Colors[ImGuiCol_SeparatorHovered] = Generate( Dark::Background::Neutral );
	Colors[ImGuiCol_SeparatorActive] = Generate( Dark::Background::Bright );

	Colors[ImGuiCol_ResizeGrip] = Colors[ImGuiCol_TitleBg];
	Colors[ImGuiCol_ResizeGripHovered] = Colors[ImGuiCol_TitleBgActive];
	Colors[ImGuiCol_ResizeGripActive] = Generate( Dark::Active::Red );

	Colors[ImGuiCol_Tab] = ImColor{ IM_COL32( 0xd6, 0x5d, 0x0e, 0xD8 ) };
	Colors[ImGuiCol_TabHovered] = ImColor{ IM_COL32( 0xfe, 0x80, 0x19, 0xCC ) };
	Colors[ImGuiCol_TabActive] = ImColor{ IM_COL32( 0xfe, 0x80, 0x19, 0xFF ) };

	Colors[ImGuiCol_TabUnfocused] = ImColor{ IM_COL32( 0x1d, 0x20, 0x21, 0.97f ) };
	Colors[ImGuiCol_TabUnfocusedActive] = ImColor{ IM_COL32( 0x1d, 0x20, 0x21, 0xFF ) };

	Colors[ImGuiCol_PlotLines] = Generate( Dark::Regular::Red );
	Colors[ImGuiCol_PlotLinesHovered] = Generate( Dark::Active::Red );

	Colors[ImGuiCol_PlotHistogram] = Generate( Dark::Regular::Blue );
	Colors[ImGuiCol_PlotHistogramHovered] = Generate( Dark::Active::Blue );

	Colors[ImGuiCol_DragDropTarget] = ImColor{ IM_COL32( 0x98, 0x97, 0x1a, 0.90f ) };
	Colors[ImGuiCol_TextSelectedBg] = ImColor{ IM_COL32( 0x45, 0x85, 0x88, 0xF0 ) };
	Colors[ImGuiCol_NavHighlight] = ImColor{ IM_COL32( 0x83, 0xa5, 0x98, 0xFF ) };
	Colors[ImGuiCol_NavWindowingHighlight] = ImColor{ IM_COL32( 0xfb, 0xf1, 0xc7, 0xB2 ) };
	Colors[ImGuiCol_NavWindowingDimBg] = ImColor{ IM_COL32( 0x7c, 0x6f, 0x64, 0x33 ) };
	Colors[ImGuiCol_ModalWindowDimBg] = Generate( Dark::Background::Default );
	Colors[ImGuiCol_ModalWindowDimBg].w = 0.65f;

	// Extra adjustments.
	Style.WindowPadding = ImVec2( 6, 4 );
	Style.WindowRounding = 4.0f;
	Style.FramePadding = ImVec2( 5, 2 );
	Style.FrameRounding = 3.0f;
	Style.ItemSpacing = ImVec2( 7, 3 );
	Style.ItemInnerSpacing = ImVec2( 1, 1 );
	Style.TouchExtraPadding = ImVec2( 0, 0 );
	Style.IndentSpacing = 6.0f;
	Style.ScrollbarSize = 12.0f;
	Style.ScrollbarRounding = 16.0f;
	Style.GrabMinSize = 20.0f;
	Style.GrabRounding = 2.0f;

	Style.WindowTitleAlign = ImVec2( 0.0f, 0.5f );
}

void ThemeGruvboxLight()
{
	ThemeDefault();

	auto& Style = ImGui::GetStyle();
	Style.ChildRounding = 0;
	Style.GrabRounding = 0;
	Style.FrameRounding = 0;
	Style.PopupRounding = 0;
	Style.ScrollbarRounding = 0;
	Style.TabRounding = 0;
	Style.WindowRounding = 0;
	Style.FramePadding = { 4, 4 };

	Style.WindowTitleAlign = { 0.5, 0.5 };

	ImVec4* Colors = ImGui::GetStyle().Colors;

	Colors[ImGuiCol_Text] = Generate( Dark::Background::Dim );
	Colors[ImGuiCol_TextDisabled] = Generate( Dark::Background::Dim );

	Colors[ImGuiCol_WindowBg] = Generate( Dark::Foreground::Dim );
	Darken( Colors[ImGuiCol_WindowBg], 0.9f );
	Colors[ImGuiCol_ChildBg] = Generate( Dark::Foreground::Dim );
	Colors[ImGuiCol_PopupBg] = Colors[ImGuiCol_WindowBg];

	Colors[ImGuiCol_Border] = Generate( Dark::Background::Default );
	Colors[ImGuiCol_BorderShadow] = Generate( Dark::Regular::Red );

	Colors[ImGuiCol_FrameBg] = Generate( Dark::Foreground::Faint );
	Colors[ImGuiCol_FrameBgHovered] = Generate( Dark::Foreground::Neutral );
	Colors[ImGuiCol_FrameBgActive] = Generate( Dark::Foreground::Bright );

	Colors[ImGuiCol_TitleBg] = Generate( Dark::Active::Red );
	Darken( Colors[ImGuiCol_TitleBg], 0.6f );

	Colors[ImGuiCol_TitleBgActive] = Generate( Dark::Active::Red );
	Colors[ImGuiCol_TitleBgCollapsed] = Generate( Dark::Active::Red );
	Darken( Colors[ImGuiCol_TitleBgCollapsed], 0.4f );

	Colors[ImGuiCol_MenuBarBg] = Generate( Dark::Foreground::Dim );

	Colors[ImGuiCol_ScrollbarBg] = Generate( Dark::Foreground::Default );
	Colors[ImGuiCol_ScrollbarGrab] = Generate( Dark::Foreground::Faint );
	Colors[ImGuiCol_ScrollbarGrabHovered] = Generate( Dark::Foreground::Neutral );
	Colors[ImGuiCol_ScrollbarGrabActive] = Generate( Dark::Background::Dim );

	Colors[ImGuiCol_CheckMark] = Generate( Dark::Regular::Yellow );

	Colors[ImGuiCol_SliderGrab] = Generate( Dark::Active::Blue );
	Colors[ImGuiCol_SliderGrabActive] = Generate( Dark::Regular::Blue );

	Colors[ImGuiCol_Button] = Generate( Dark::Regular::Aqua );
	Colors[ImGuiCol_Button].w = 0.25f;
	Colors[ImGuiCol_ButtonHovered] = Generate( Dark::Regular::Blue );
	Colors[ImGuiCol_ButtonActive] = Generate( Dark::Active::Yellow );

	Colors[ImGuiCol_Header] = Generate( Dark::Regular::Blue );
	Colors[ImGuiCol_Header].w = 0.65f;
	Colors[ImGuiCol_HeaderHovered] = Generate( Dark::Foreground::Bright );
	Colors[ImGuiCol_HeaderHovered].w = 0.35f;
	Colors[ImGuiCol_HeaderActive] = Generate( Dark::Foreground::Bright );

	Colors[ImGuiCol_Separator] = Generate( Dark::Foreground::Faint );
	Colors[ImGuiCol_SeparatorHovered] = Generate( Dark::Foreground::Neutral );
	Colors[ImGuiCol_SeparatorActive] = Generate( Dark::Foreground::Bright );

	Colors[ImGuiCol_ResizeGrip] = Colors[ImGuiCol_TitleBg];
	Colors[ImGuiCol_ResizeGripHovered] = Colors[ImGuiCol_TitleBgActive];
	Colors[ImGuiCol_ResizeGripActive] = Generate( Dark::Regular::Red );

	Colors[ImGuiCol_Tab] = ImColor{ IM_COL32( 0xd6, 0x5d, 0x0e, 0xD8 ) };
	Colors[ImGuiCol_TabHovered] = ImColor{ IM_COL32( 0xfe, 0x80, 0x19, 0xCC ) };
	Colors[ImGuiCol_TabActive] = ImColor{ IM_COL32( 0xfe, 0x80, 0x19, 0xFF ) };

	Colors[ImGuiCol_TabUnfocused] = ImColor{ IM_COL32( 0x1d, 0x20, 0x21, 0.97f ) };
	Colors[ImGuiCol_TabUnfocusedActive] = ImColor{ IM_COL32( 0x1d, 0x20, 0x21, 0xFF ) };

	Colors[ImGuiCol_PlotLines] = Generate( Dark::Active::Red );
	Colors[ImGuiCol_PlotLinesHovered] = Generate( Dark::Regular::Red );

	Colors[ImGuiCol_PlotHistogram] = Generate( Dark::Active::Blue );
	Colors[ImGuiCol_PlotHistogramHovered] = Generate( Dark::Regular::Blue );

	Colors[ImGuiCol_DragDropTarget] = ImColor{ IM_COL32( 0x98, 0x97, 0x1a, 0.90f ) };
	Colors[ImGuiCol_TextSelectedBg] = ImColor{ IM_COL32( 0x45, 0x85, 0x88, 0xF0 ) };
	Colors[ImGuiCol_NavHighlight] = ImColor{ IM_COL32( 0x83, 0xa5, 0x98, 0xFF ) };
	Colors[ImGuiCol_NavWindowingHighlight] = ImColor{ IM_COL32( 0xfb, 0xf1, 0xc7, 0xB2 ) };
	Colors[ImGuiCol_NavWindowingDimBg] = ImColor{ IM_COL32( 0x7c, 0x6f, 0x64, 0x33 ) };
	Colors[ImGuiCol_ModalWindowDimBg] = Generate( Dark::Foreground::Default );
	Colors[ImGuiCol_ModalWindowDimBg].w = 0.65f;

	// Extra adjustments.
	Style.WindowPadding = ImVec2( 6, 4 );
	Style.WindowRounding = 4.0f;
	Style.FramePadding = ImVec2( 5, 2 );
	Style.FrameRounding = 3.0f;
	Style.ItemSpacing = ImVec2( 7, 3 );
	Style.ItemInnerSpacing = ImVec2( 1, 1 );
	Style.TouchExtraPadding = ImVec2( 0, 0 );
	Style.IndentSpacing = 6.0f;
	Style.ScrollbarSize = 12.0f;
	Style.ScrollbarRounding = 16.0f;
	Style.GrabMinSize = 20.0f;
	Style.GrabRounding = 2.0f;

	Style.WindowTitleAlign = ImVec2( 0.0f, 0.5f );
}