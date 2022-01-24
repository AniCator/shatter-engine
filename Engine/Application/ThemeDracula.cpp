// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include "ThemeDefault.h"
#include <ThirdParty/imgui-1.70/imgui.h>

void ThemeDracula()
{
	ThemeDefault();

	ImGuiStyle& Style = ImGui::GetStyle();
	Style.WindowRounding = 5.3f;
	Style.GrabRounding = Style.FrameRounding = 2.3f;
	Style.ScrollbarRounding = 5.0f;
	Style.ItemSpacing.y = 6.5f;

	Style.Colors[ImGuiCol_Text] = { 0.73333335f, 0.73333335f, 0.73333335f, 1.00f };
	Style.Colors[ImGuiCol_TextDisabled] = { 0.34509805f, 0.34509805f, 0.34509805f, 1.00f };
	Style.Colors[ImGuiCol_WindowBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
	Style.Colors[ImGuiCol_PopupBg] = { 0.23529413f, 0.24705884f, 0.25490198f, 0.94f };
	Style.Colors[ImGuiCol_Border] = { 0.33333334f, 0.33333334f, 0.33333334f, 0.50f };
	Style.Colors[ImGuiCol_BorderShadow] = { 0.15686275f, 0.15686275f, 0.15686275f, 0.00f };
	Style.Colors[ImGuiCol_FrameBg] = { 0.16862746f, 0.16862746f, 0.16862746f, 0.54f };
	Style.Colors[ImGuiCol_FrameBgHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
	Style.Colors[ImGuiCol_FrameBgActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
	Style.Colors[ImGuiCol_TitleBg] = { 0.04f, 0.04f, 0.04f, 1.00f };
	Style.Colors[ImGuiCol_TitleBgCollapsed] = { 0.16f, 0.29f, 0.48f, 1.00f };
	Style.Colors[ImGuiCol_TitleBgActive] = { 0.00f, 0.00f, 0.00f, 0.51f };
	Style.Colors[ImGuiCol_MenuBarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.80f };
	Style.Colors[ImGuiCol_ScrollbarBg] = { 0.27058825f, 0.28627452f, 0.2901961f, 0.60f };
	Style.Colors[ImGuiCol_ScrollbarGrab] = { 0.21960786f, 0.30980393f, 0.41960788f, 0.51f };
	Style.Colors[ImGuiCol_ScrollbarGrabHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
	Style.Colors[ImGuiCol_ScrollbarGrabActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 0.91f };
	Style.Colors[ImGuiCol_CheckMark] = { 0.90f, 0.90f, 0.90f, 0.83f };
	Style.Colors[ImGuiCol_SliderGrab] = { 0.70f, 0.70f, 0.70f, 0.62f };
	Style.Colors[ImGuiCol_SliderGrabActive] = { 0.30f, 0.30f, 0.30f, 0.84f };
	Style.Colors[ImGuiCol_Button] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.49f };
	Style.Colors[ImGuiCol_ButtonHovered] = { 0.21960786f, 0.30980393f, 0.41960788f, 1.00f };
	Style.Colors[ImGuiCol_ButtonActive] = { 0.13725491f, 0.19215688f, 0.2627451f, 1.00f };
	Style.Colors[ImGuiCol_Header] = { 0.33333334f, 0.3529412f, 0.36078432f, 0.53f };
	Style.Colors[ImGuiCol_HeaderHovered] = { 0.453125f, 0.67578125f, 0.99609375f, 0.67f };
	Style.Colors[ImGuiCol_HeaderActive] = { 0.47058827f, 0.47058827f, 0.47058827f, 0.67f };
	Style.Colors[ImGuiCol_Separator] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	Style.Colors[ImGuiCol_SeparatorHovered] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	Style.Colors[ImGuiCol_SeparatorActive] = { 0.31640625f, 0.31640625f, 0.31640625f, 1.00f };
	Style.Colors[ImGuiCol_ResizeGrip] = { 1.00f, 1.00f, 1.00f, 0.85f };
	Style.Colors[ImGuiCol_ResizeGripHovered] = { 1.00f, 1.00f, 1.00f, 0.60f };
	Style.Colors[ImGuiCol_ResizeGripActive] = { 1.00f, 1.00f, 1.00f, 0.90f };
	Style.Colors[ImGuiCol_PlotLines] = { 0.61f, 0.61f, 0.61f, 1.00f };
	Style.Colors[ImGuiCol_PlotLinesHovered] = { 1.00f, 0.43f, 0.35f, 1.00f };
	Style.Colors[ImGuiCol_PlotHistogram] = { 0.90f, 0.70f, 0.00f, 1.00f };
	Style.Colors[ImGuiCol_PlotHistogramHovered] = { 1.00f, 0.60f, 0.00f, 1.00f };
	Style.Colors[ImGuiCol_TextSelectedBg] = { 0.18431373f, 0.39607847f, 0.79215693f, 0.90f };
}