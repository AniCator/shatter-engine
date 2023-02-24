// Copyright © 2017, Christiaan Bakker, All rights reserved.
#include "GradeEvent.h"

#include <Engine/Application/ApplicationMenu.h>
#include <Engine/Utility/Serialize.h>

#include <ThirdParty/imgui-1.70/imgui.h>

void GradeEvent::Execute()
{
	GetRenderer().Grade.Blend( Grade );
}

void GradeEvent::Context()
{
	ImGui::Text( "Grade" );
	ImGui::DragFloat( "Weight", &Grade.Weight, 0.01f, 0.0f, 1.0f, "%.2f" );

	int Priority = Grade.Priority;
	if( ImGui::DragInt( "Priority", &Priority, 1.0f, 0, 1000 ) )
	{
		Grade.Priority = Priority;
	}

	PopulateGradePanel( Grade );
}

constexpr const char* GradeName = "Grade";
const char* GradeEvent::GetName()
{
	return GradeName;
}

const char* GradeEvent::GetType() const
{
	return GradeName;
}

void GradeEvent::Export( CData& Data ) const
{
	Serialize::Export( Data, "t", Grade.Tint );
	Serialize::Export( Data, "l", Grade.Lift );
	Serialize::Export( Data, "gm", Grade.Gamma );
	Serialize::Export( Data, "gn", Grade.Gain );
	Serialize::Export( Data, "w", Grade.Weight );
	Serialize::Export( Data, "p", Grade.Priority );
}

void GradeEvent::Import( CData& Data )
{
	Serialize::Import( Data, "t", Grade.Tint );
	Serialize::Import( Data, "l", Grade.Lift );
	Serialize::Import( Data, "gm", Grade.Gamma );
	Serialize::Import( Data, "gn", Grade.Gain );
	Serialize::Import( Data, "w", Grade.Weight );
	Serialize::Import( Data, "p", Grade.Priority );
}
