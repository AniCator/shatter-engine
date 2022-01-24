// Copyright © 2017, Christiaan Bakker, All rights reserved.

#include <ThirdParty/imgui-1.70/imgui.h>

bool DefaultStyleValid = false;
ImGuiStyle DefaultStyle;

void ThemeDefault()
{
	ImGuiStyle& Style = ImGui::GetStyle();
	if( !DefaultStyleValid )
	{
		DefaultStyle = Style;
		DefaultStyleValid = true;
	}

	Style = DefaultStyle;
}