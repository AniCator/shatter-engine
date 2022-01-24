// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include "ThemeDefault.h"
#include <ThirdParty/imgui-1.70/imgui.h>

auto CherryHigh = [] ( float v ) { return ImVec4( 0.502f, 0.075f, 0.256f, v ); };
auto CherryMedium = [] ( float v ) { return ImVec4( 0.455f, 0.198f, 0.301f, v ); };
auto CherryLow = [] ( float v ) { return ImVec4( 0.232f, 0.201f, 0.271f, v ); };

auto CherryBackground = [] ( float v ) { return ImVec4( 0.200f, 0.220f, 0.270f, v ); };
auto CherryText = [] ( float v ) { return ImVec4( 0.860f, 0.930f, 0.890f, v ); };

void ThemeCherry()
{
	ThemeDefault();

	ImGuiStyle& Style = ImGui::GetStyle();
	Style.Colors[ImGuiCol_Text] = CherryText( 0.78f );
	Style.Colors[ImGuiCol_TextDisabled] = CherryText( 0.28f );
	Style.Colors[ImGuiCol_WindowBg] = ImVec4( 0.13f, 0.14f, 0.17f, 1.00f );
	// Style.Colors[ImGuiCol_ChildWindowBg] = CherryBackground( 0.58f );
	// Style.Colors[ImGuiCol_PopupBg] = CherryBackground( 0.9f );
	Style.Colors[ImGuiCol_ChildWindowBg] = Style.Colors[ImGuiCol_WindowBg];
	Style.Colors[ImGuiCol_PopupBg] = Style.Colors[ImGuiCol_WindowBg];
	Style.Colors[ImGuiCol_Border] = ImVec4( 0.31f, 0.31f, 1.00f, 0.00f );
	Style.Colors[ImGuiCol_BorderShadow] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
	Style.Colors[ImGuiCol_FrameBg] = CherryBackground( 1.00f );
	Style.Colors[ImGuiCol_FrameBgHovered] = CherryMedium( 0.78f );
	Style.Colors[ImGuiCol_FrameBgActive] = CherryMedium( 1.00f );
	Style.Colors[ImGuiCol_TitleBg] = CherryLow( 1.00f );
	Style.Colors[ImGuiCol_TitleBgActive] = CherryHigh( 1.00f );
	Style.Colors[ImGuiCol_TitleBgCollapsed] = CherryBackground( 0.75f );
	Style.Colors[ImGuiCol_MenuBarBg] = CherryBackground( 0.47f );
	Style.Colors[ImGuiCol_ScrollbarBg] = CherryBackground( 1.00f );
	Style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4( 0.09f, 0.15f, 0.16f, 1.00f );
	Style.Colors[ImGuiCol_ScrollbarGrabHovered] = CherryMedium( 0.78f );
	Style.Colors[ImGuiCol_ScrollbarGrabActive] = CherryMedium( 1.00f );
	Style.Colors[ImGuiCol_CheckMark] = ImVec4( 0.71f, 0.22f, 0.27f, 1.00f );
	Style.Colors[ImGuiCol_SliderGrab] = ImVec4( 0.47f, 0.77f, 0.83f, 0.14f );
	Style.Colors[ImGuiCol_SliderGrabActive] = ImVec4( 0.71f, 0.22f, 0.27f, 1.00f );
	Style.Colors[ImGuiCol_Button] = ImVec4( 0.47f, 0.77f, 0.83f, 0.14f );
	Style.Colors[ImGuiCol_ButtonHovered] = CherryMedium( 0.86f );
	Style.Colors[ImGuiCol_ButtonActive] = CherryMedium( 1.00f );
	Style.Colors[ImGuiCol_Header] = CherryMedium( 0.76f );
	Style.Colors[ImGuiCol_HeaderHovered] = CherryMedium( 0.86f );
	Style.Colors[ImGuiCol_HeaderActive] = CherryHigh( 1.00f );
	Style.Colors[ImGuiCol_Column] = ImVec4( 0.14f, 0.16f, 0.19f, 1.00f );
	Style.Colors[ImGuiCol_ColumnHovered] = CherryMedium( 0.78f );
	Style.Colors[ImGuiCol_ColumnActive] = CherryMedium( 1.00f );
	Style.Colors[ImGuiCol_ResizeGrip] = ImVec4( 0.47f, 0.77f, 0.83f, 0.04f );
	Style.Colors[ImGuiCol_ResizeGripHovered] = CherryMedium( 0.78f );
	Style.Colors[ImGuiCol_ResizeGripActive] = CherryMedium( 1.00f );
	Style.Colors[ImGuiCol_PlotLines] = CherryText( 0.63f );
	Style.Colors[ImGuiCol_PlotLinesHovered] = CherryMedium( 1.00f );
	Style.Colors[ImGuiCol_PlotHistogram] = CherryText( 0.63f );
	Style.Colors[ImGuiCol_PlotHistogramHovered] = CherryMedium( 1.00f );
	Style.Colors[ImGuiCol_TextSelectedBg] = CherryMedium( 0.43f );
	// [...]
	Style.Colors[ImGuiCol_ModalWindowDarkening] = CherryBackground( 0.73f );

	Style.WindowPadding = ImVec2( 6, 4 );
	Style.WindowRounding = 0.0f;
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

	Style.WindowTitleAlign.x = 0.50f;

	Style.Colors[ImGuiCol_Border] = ImVec4( 0.539f, 0.479f, 0.255f, 0.162f );
}