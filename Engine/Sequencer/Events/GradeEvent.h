// Copyright © 2017, Christiaan Bakker, All rights reserved.
#pragma once

#include <Engine/Display/Rendering/Renderer.h>
#include <Engine/Sequencer/Sequencer.h>

struct GradeEvent : TrackEvent
{
	void Execute() override;
	void Reset() override {};
	void Context() override;
	const char* GetName() override;
	const char* GetType() const override;
	void Export( CData& Data ) const override;
	void Import( CData& Data ) override;

protected:
	ColorGrade Grade;
};